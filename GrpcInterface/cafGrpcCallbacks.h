// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
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
#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/sync_stream.h>

#include <string>

namespace caffa::rpc
{
/**
 * Non-templated Base class for all caf grpc callbacks
 */
class AbstractGrpcCallback
{
public:
    enum CallState
    {
        CREATE,
        PROCESS_REQUEST,
        FINISH_REQUEST,
    };

public:
    AbstractGrpcCallback();

    virtual ~AbstractGrpcCallback() {}
    virtual std::string           name() const                                                         = 0;
    virtual AbstractGrpcCallback* emptyClone() const                                                   = 0;
    virtual void                  createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) = 0;
    virtual void                  onProcessRequest()                                                   = 0;
    virtual void                  onFinishRequest() {}

    inline CallState           callState() const;
    inline const grpc::Status& status() const;
    inline void                setNextCallState( CallState state );

protected:
    CallState    m_state;
    grpc::Status m_status;
};

/**
 * Templated callback for any grpc service
 */
template <typename ServiceT, typename RequestT, typename ReplyT>
class GrpcServiceCallback : public AbstractGrpcCallback
{
public:
    using ResponseWriterT = grpc::ServerAsyncResponseWriter<ReplyT>;
    using MethodImplT     = std::function<grpc::Status( ServiceT&, grpc::ServerContext*, const RequestT*, ReplyT* )>;
    using MethodRequestT  = std::function<
        void( ServiceT&, grpc::ServerContext*, RequestT*, ResponseWriterT*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void* )>;

    GrpcServiceCallback( ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest );

    std::string     name() const override;
    const RequestT& request() const;
    ReplyT&         reply();

    AbstractGrpcCallback* emptyClone() const override;
    void                  createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) override;
    void                  onProcessRequest() override;

protected:
    ServiceT* m_service;
    RequestT  m_request;
    ReplyT    m_reply;

    grpc::ServerContext m_context;
    ResponseWriterT     m_responder;
    MethodImplT         m_methodImpl;
    MethodRequestT      m_methodRequest;
};

} // namespace caffa::rpc
#include "cafGrpcCallbacks.inl"
