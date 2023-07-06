// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
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
ServiceCallback<ServiceT, RequestT, ReplyT>::ServiceCallback( ServiceT*      service,
                                                              MethodImplT    methodImpl,
                                                              MethodRequestT methodRequest )
    : m_service( service )
    , m_responder( &m_context )
    , m_methodImpl( methodImpl )
    , m_methodRequest( methodRequest )
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
    ss << typeid( ReplyT ).name() << " " << typeid( ServiceT ).name() << ":" << typeid( RequestT ).name();

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
AbstractCallback* ServiceCallback<ServiceT, RequestT, ReplyT>::emptyClone() const
{
    return new ServiceCallback<ServiceT, RequestT, ReplyT>( this->m_service, this->m_methodImpl, this->m_methodRequest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ServiceCallback<ServiceT, RequestT, ReplyT>::createRequestHandler( grpc::ServerCompletionQueue* completionQueue )
{
    // The Request-method is where the service gets registered to respond to a given request.
    m_methodRequest( *this->m_service, &m_context, &this->m_request, &m_responder, completionQueue, completionQueue, this );
    // Simple Service requests don't need initialisation, so proceed to process as soon as a request turns up.
    this->setNextCallState( AbstractCallback::PROCESS_REQUEST );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ServiceT, typename RequestT, typename ReplyT>
void ServiceCallback<ServiceT, RequestT, ReplyT>::onProcessRequest()
{
    // Call request handler method
    this->m_status = m_methodImpl( *this->m_service, &m_context, &this->m_request, &this->m_reply );
    // Simply Service requests are finished as soon as you've done any processing.
    // So next time we receive a new tag on the command queue we should proceed to finish.
    this->setNextCallState( AbstractCallback::FINISH_REQUEST );
    // Finish will push this callback back on the command queue (now with Finish as the call state).
    m_responder.Finish( this->m_reply, this->m_status, this );
}

} // namespace caffa::rpc
