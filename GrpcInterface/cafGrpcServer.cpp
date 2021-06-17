///////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cafGrpcServer.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcServiceInterface.h"

#include "cafAssert.h"
#include "cafLogger.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <thread>

using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

using namespace std::chrono_literals;

namespace caffa::rpc
{
//==================================================================================================
//
// The GRPC server implementation
//
//==================================================================================================
class ServerImpl
{
public:
    ServerImpl( int portNumber )
        : m_portNumber( portNumber )
        , m_receivedQuitRequest( false )
    {
    }

    ~ServerImpl() {}

    void run()
    {
        m_thread = std::thread( &ServerImpl::initializeAndWaitForNextRequest, this );

        m_thread.join();
        CAFFA_DEBUG( "Request handling thread joined" );

        cleanup();
    }

    void quit()
    {
        CAFFA_DEBUG( "Received quit request" );
        m_receivedQuitRequest = true;
    }

    bool quitting() const
    {
        std::lock_guard<std::mutex> launchLock( m_appStateMutex );
        return m_receivedQuitRequest;
    }

    void cleanup()
    {
        CAFFA_DEBUG( "Shutting down" );
        if ( m_server )
        {
            CAFFA_DEBUG( "Processing requests" );

            // Must destroy server before services
            m_server.reset();
            CAFFA_TRACE( "Attempting to clear queue" );
            m_completionQueue.reset();

            CAFFA_TRACE( "Attempting to clear services" );
            // Finally clear services
            m_services.clear();
        }
        CAFFA_DEBUG( "Finished shutting down" );
    }

    int port() const { return m_portNumber; }

    bool running() const
    {
        std::lock_guard<std::mutex> launchLock( m_appStateMutex );
        return m_server != nullptr;
    }

    void initialize()
    {
        CAFFA_ASSERT( m_portNumber > 0 && m_portNumber <= (int)std::numeric_limits<uint16_t>::max() );

        std::string serverAddress = "0.0.0.0:" + std::to_string( m_portNumber );
        CAFFA_DEBUG( "Initialising new server with address: " << serverAddress );

        ServerBuilder builder;
        builder.AddListeningPort( serverAddress, grpc::InsecureServerCredentials() );

        for ( auto key : ServiceFactory::instance()->allKeys() )
        {
            std::shared_ptr<ServiceInterface> service( ServiceFactory::instance()->create( key ) );
            auto                              grpcService = dynamic_cast<grpc::Service*>( service.get() );
            CAFFA_ASSERT( grpcService );
            builder.RegisterService( grpcService );
            m_services.push_back( service );
        }

        m_completionQueue = builder.AddCompletionQueue();
        m_server          = builder.BuildAndStart();

        CAFFA_ASSERT( m_server );

        // Spawn new CallData instances to serve new clients.
        for ( auto service : m_services )
        {
            for ( auto callback : service->createCallbacks() )
            {
                process( callback );
            }
        }
    }

    void initializeAndWaitForNextRequest()
    {
        initialize();
        waitForNextRequest();
    }

private:
    void waitForNextRequest()
    {
        void* tag;
        bool  ok = false;

        CAFFA_DEBUG( "Waiting for requests" );
        while ( bool nextStatus = m_completionQueue->Next( &tag, &ok ) )
        {
            CAFFA_TRACE( "Received request with status: " << nextStatus );

            AbstractCallback* method = static_cast<AbstractCallback*>( tag );
            if ( ok )
            {
                process( method );
            }
            if ( quitting() )
            {
                // Shutdown server and queue
                m_server->Shutdown();
                m_completionQueue->Shutdown();
            }
        }
        CAFFA_DEBUG( "Request handler quitting" );
    }

    void process( AbstractCallback* method )
    {
        if ( method->callState() == AbstractCallback::CREATE )
        {
            CAFFA_TRACE( "Create request handler: " << method->name() );
            method->createRequestHandler( m_completionQueue.get() );
            CAFFA_TRACE( "Request handler created" );
        }
        else if ( method->callState() == AbstractCallback::INIT_REQUEST_STARTED )
        {
            CAFFA_TRACE( "Init request: " << method->name() );
            method->onInitRequestStarted();
        }
        else if ( method->callState() == AbstractCallback::INIT_REQUEST_COMPLETED )
        {
            CAFFA_TRACE( "Init request completed: " << method->name() );
            method->onInitRequestCompleted();
        }
        else if ( method->callState() == AbstractCallback::PROCESS_REQUEST )
        {
            CAFFA_TRACE( "Processing request: " << method->name() );
            method->onProcessRequest();
        }
        else
        {
            CAFFA_TRACE( "Finishing request: " << method->name() );
            method->onFinishRequest();
            if ( !quitting() )
            {
                process( method->emptyClone() );
            }
            delete method;
        }
    }

private:
    int                                          m_portNumber;
    std::unique_ptr<grpc::ServerCompletionQueue> m_completionQueue;
    std::unique_ptr<grpc::Server>                m_server;
    std::list<std::shared_ptr<ServiceInterface>> m_services;
    mutable std::mutex                           m_appStateMutex;
    std::thread                                  m_thread;

    bool m_receivedQuitRequest;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Server::Server( int portNumber )
{
    m_serverImpl = new ServerImpl( portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Server::~Server()
{
    delete m_serverImpl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int Server::port() const
{
    if ( m_serverImpl ) return m_serverImpl->port();

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Server::running() const
{
    if ( m_serverImpl ) return m_serverImpl->running();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Server::run()
{
    m_serverImpl->run();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Server::initialize()
{
    m_serverImpl->initialize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Server::quit()
{
    if ( m_serverImpl ) m_serverImpl->quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Server::cleanup()
{
    if ( m_serverImpl ) m_serverImpl->cleanup();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Server::quitting() const
{
    if ( m_serverImpl ) return m_serverImpl->quitting();

    return false;
}

} // namespace caffa::rpc
