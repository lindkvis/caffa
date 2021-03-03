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

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include <thread>

using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

namespace caf::rpc
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

    ~ServerImpl() { forceQuit(); }

    void run() { m_thread = std::thread( &ServerImpl::initializeAndWaitForNextRequest, this ); }

    void quit()
    {
        std::lock_guard<std::mutex> launchLock( m_appStateMutex );
        m_receivedQuitRequest = true;
    }

    bool quitting() const
    {
        std::lock_guard<std::mutex> launchLock( m_appStateMutex );
        return m_receivedQuitRequest;
    }

    void forceQuit()
    {
        if ( m_server )
        {
            // Clear unhandled requests
            while ( !m_queuedRequests.empty() )
            {
                AbstractCallback* method = m_queuedRequests.front();
                m_queuedRequests.pop_front();
                delete method;
            }

            // Shutdown server and queue
            m_server->Shutdown();
            m_completionQueue->Shutdown();

            // Wait for thread to join after handling the shutdown call
            m_thread.join();

            // Must destroy server before services
            m_server.reset();
            m_completionQueue.reset();


            // Finally clear services
            m_services.clear();
        }
    }

    int port() const { return m_portNumber; }

    bool running() const
    {
        std::lock_guard<std::mutex> launchLock( m_appStateMutex );
        return m_server != nullptr;
    }

    void initialize()
    {
        CAF_ASSERT( m_portNumber > 0 && m_portNumber <= (int)std::numeric_limits<uint16_t>::max() );

        std::string serverAddress = "localhost:" + std::to_string( m_portNumber );

        ServerBuilder builder;
        builder.AddListeningPort( serverAddress, grpc::InsecureServerCredentials() );

        for ( auto key : ServiceFactory::instance()->allKeys() )
        {
            std::shared_ptr<ServiceInterface> service( ServiceFactory::instance()->create( key ) );
            builder.RegisterService( dynamic_cast<grpc::Service*>( service.get() ) );
            m_services.push_back( service );
        }

        m_completionQueue = builder.AddCompletionQueue();
        m_server          = builder.BuildAndStart();

        CAF_ASSERT( m_server );

        // Spawn new CallData instances to serve new clients.
        for ( auto service : m_services )
        {
            for ( auto callback : service->registerCallbacks() )
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

    size_t processAllRequests()
    {
        std::list<AbstractCallback*> waitingRequests;
        {
            // Block only while transferring the unprocessed requests to a local function list
            std::lock_guard<std::mutex> requestLock( m_requestMutex );
            waitingRequests.swap( m_queuedRequests );
        }
        size_t count = waitingRequests.size();

        // Now free to receive new requests from client while processing the current ones.
        while ( !waitingRequests.empty() )
        {
            AbstractCallback* method = waitingRequests.front();
            waitingRequests.pop_front();
            process( method );
        }
        return count;
    }

private:
    void waitForNextRequest()
    {
        void* tag;
        bool  ok = false;
        {
            std::lock_guard<std::mutex> requestLock( m_appStateMutex );
        }
        while ( m_completionQueue->Next( &tag, &ok ) )
        {
            std::lock_guard<std::mutex> requestLock( m_requestMutex );
            AbstractCallback*           method = static_cast<AbstractCallback*>( tag );
            if ( !ok )
            {
                method->setNextCallState( AbstractCallback::FINISH_REQUEST );
            }
            m_queuedRequests.push_back( method );
        }
    }

    void process( AbstractCallback* method )
    {
        if ( method->callState() == AbstractCallback::CREATE )
        {
            method->createRequestHandler( m_completionQueue.get() );
        }
        else if ( method->callState() == AbstractCallback::INIT_REQUEST_STARTED )
        {
            method->onInitRequestStarted();
        }
        else if ( method->callState() == AbstractCallback::INIT_REQUEST_COMPLETED )
        {
            method->onInitRequestCompleted();
        }
        else if ( method->callState() == AbstractCallback::PROCESS_REQUEST )
        {
            method->onProcessRequest();
        }
        else
        {
            method->onFinishRequest();
            process( method->emptyClone() );
            delete method;
        }
    }

private:
    int                                          m_portNumber;
    std::unique_ptr<grpc::ServerCompletionQueue> m_completionQueue;
    std::unique_ptr<grpc::Server>                m_server;
    std::list<std::shared_ptr<ServiceInterface>> m_services;
    std::list<AbstractCallback*>                 m_queuedRequests;
    std::mutex                                   m_requestMutex;
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
size_t Server::processAllRequests()
{
    return m_serverImpl->processAllRequests();
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
void Server::forceQuit()
{
    if ( m_serverImpl ) m_serverImpl->forceQuit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Server::quitting() const
{
    if ( m_serverImpl ) return m_serverImpl->quitting();

    return false;
}

} // namespace caf::rpc
