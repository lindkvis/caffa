//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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
//

#include "cafAssert.h"

#include <sstream>

namespace caffa::rpc
{
inline AbstractCallback::AbstractCallback()
    : m_state( CREATE )
    , m_status( grpc::Status::OK )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AbstractCallback::CallState AbstractCallback::callState() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const grpc::Status& AbstractCallback::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
inline void AbstractCallback::setNextCallState( CallState state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
ServiceCallback<ServiceT, RequestT, ReplyT>::ServiceCallback( ServiceT* service )
    : m_service( service )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
std::string ServiceCallback<ServiceT, RequestT, ReplyT>::name() const
{
    std::string        fullName;
    std::ostringstream ss;
    ss << typeid( ServiceT ).name() << ":" << methodType() << "(" << typeid( RequestT ).name() << ", "
       << typeid( ReplyT ).name() << ")";

    return ss.str();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
const RequestT& ServiceCallback<ServiceT, RequestT, ReplyT>::request() const
{
    return m_request;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
ReplyT& ServiceCallback<ServiceT, RequestT, ReplyT>::reply()
{
    return m_reply;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
UnaryCallback<ServiceT, RequestT, ReplyT>::UnaryCallback( ServiceT*      service,
                                                          MethodImplT    methodImpl,
                                                          MethodRequestT methodRequest )
    : ServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_responder( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
AbstractCallback* UnaryCallback<ServiceT, RequestT, ReplyT>::emptyClone() const
{
    return new UnaryCallback<ServiceT, RequestT, ReplyT>( this->m_service, this->m_methodImpl, this->m_methodRequest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void UnaryCallback<ServiceT, RequestT, ReplyT>::createRequestHandler( grpc::ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this );
    // Simple unary requests don't need initialisation, so proceed to process as soon as a request turns up.
    this->setNextCallState( AbstractCallback::PROCESS_REQUEST );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void UnaryCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply );
    // Simply unary requests are finished as soon as you've done any processing.
    // So next time we receive a new tag on the command queue we should proceed to finish.
    this->setNextCallState( AbstractCallback::FINISH_REQUEST );
    // Finish will push this callback back on the command queue (now with Finish as the call state).
    m_responder.Finish( this->m_reply, this->m_status, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
std::string UnaryCallback<ServiceT, RequestT, ReplyT>::methodType() const
{
    return "RegularMethod";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::ServerToClientStreamCallback( ServiceT*      service,
                                                                                        MethodImplT    methodImpl,
                                                                                        MethodRequestT methodRequest,
                                                                                        StateHandler<RequestT>* stateHandler )
    : ServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_responder( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
    , m_stateHandler( stateHandler )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
AbstractCallback* ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::emptyClone() const
{
    return new ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>( this->m_service,
                                                                         m_methodImpl,
                                                                         m_methodRequest,
                                                                         m_stateHandler->emptyClone() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::createRequestHandler( grpc::ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this );
    // Server->Client Streaming requests require initialisation. However, we receive the complete request immediately.
    // So can proceed directly to completion of the init request.
    this->setNextCallState( AbstractCallback::INIT_REQUEST_COMPLETED );
}

//--------------------------------------------------------------------------------------------------
/// Perform initialisation tasks at the time of receiving a complete request
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::onInitRequestCompleted()
{
    // Initialise streaming state handler
    this->m_status = m_stateHandler->init( &this->m_request );

    if ( !this->m_status.ok() )
    {
        // We have an error. Proceed to finish and report the status
        this->setNextCallState( AbstractCallback::FINISH_REQUEST );
        m_responder.Finish( this->m_status, this );
        return;
    }

    // Move on to processing and perform the first processing immediately since the client will
    // not request anything more.
    this->setNextCallState( AbstractCallback::PROCESS_REQUEST );
    this->onProcessRequest();
}

//--------------------------------------------------------------------------------------------------
/// Process a streaming request and send one package
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get() );

    if ( this->m_status.ok() )
    {
        // The write call will send data to client AND put this callback back on the command queue
        // so that this method gets called again to send the next stream package.
        m_responder.Write( this->m_reply, this );
    }
    else
    {
        this->setNextCallState( AbstractCallback::FINISH_REQUEST );
        // Out of range means we're finished but it isn't an error
        if ( this->m_status.error_code() == grpc::OUT_OF_RANGE )
        {
            this->m_status = grpc::Status::OK;
        }
        // Finish will put this callback back on the command queue, now with a finish state.
        m_responder.Finish( this->m_status, this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
std::string ServerToClientStreamCallback<ServiceT, RequestT, ReplyT>::methodType() const
{
    return "StreamingMethod";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::ClientToServerStreamCallback( ServiceT*      service,
                                                                                        MethodImplT    methodImpl,
                                                                                        MethodRequestT methodRequest,
                                                                                        StateHandler<RequestT>* stateHandler )
    : ServiceCallback<ServiceT, RequestT, ReplyT>( service )
    , m_reader( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
    , m_stateHandler( stateHandler )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
AbstractCallback* ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::emptyClone() const
{
    return new ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>( this->m_service,
                                                                         m_methodImpl,
                                                                         m_methodRequest,
                                                                         m_stateHandler->emptyClone() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::createRequestHandler( grpc::ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_reader, completionQueue, completionQueue, this );
    // The client->server streaming requires initialisation and each request package is streamed asynchronously
    // So we need to start and complete the init request.
    this->setNextCallState( AbstractCallback::INIT_REQUEST_STARTED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::onInitRequestStarted()
{
    this->setNextCallState( AbstractCallback::INIT_REQUEST_COMPLETED );
    // The read call will start reading the request data and push this callback back onto the command queue
    // when the read call is completed.
    m_reader.Read( &this->m_request, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::onInitRequestCompleted()
{
    this->setNextCallState( AbstractCallback::PROCESS_REQUEST );
    // Fully received the stream package so can now init
    this->m_status = m_stateHandler->init( &this->m_request );

    if ( !this->m_status.ok() )
    {
        // We have an error. Proceed to finish and report the status
        m_reader.FinishWithError( this->m_status, this );
        this->setNextCallState( AbstractCallback::FINISH_REQUEST );
        return;
    }

    // Start reading and push this back onto the command queue.
    m_reader.Read( &this->m_request, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    this->m_reply = ReplyT(); // Make sure it is reset

    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply, m_stateHandler.get() );

    if ( !this->m_status.ok() )
    {
        this->setNextCallState( AbstractCallback::FINISH_REQUEST );
        m_reader.FinishWithError( this->m_status, this );
    }
    else
    {
        CAFFA_ASSERT( m_stateHandler->streamedValueCount() <= m_stateHandler->totalValueCount() );
        if ( m_stateHandler->streamedValueCount() == m_stateHandler->totalValueCount() )
        {
            m_stateHandler->finish();

            this->setNextCallState( AbstractCallback::FINISH_REQUEST );
            m_reader.Finish( this->m_reply, grpc::Status::OK, this );
        }
        else
        {
            m_reader.Read( &this->m_request, this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
std::string ClientToServerStreamCallback<ServiceT, RequestT, ReplyT>::methodType() const
{
    return "ClientStreamingMethod";
}

} // namespace caffa::rpc
