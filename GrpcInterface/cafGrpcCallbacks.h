//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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
class AbstractCallback
{
public:
    enum CallState
    {
        CREATE,
        INIT_REQUEST_STARTED,
        INIT_REQUEST_COMPLETED,
        PROCESS_REQUEST,
        FINISH_REQUEST
    };

public:
    AbstractCallback();

    virtual ~AbstractCallback() {}
    virtual std::string       name() const                                                         = 0;
    virtual AbstractCallback* emptyClone() const                                                   = 0;
    virtual void              createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) = 0;
    virtual void              onInitRequestStarted() {}
    virtual void              onInitRequestCompleted() {}
    virtual void              onProcessRequest() = 0;
    virtual void              onFinishRequest() {}

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
class ServiceCallback : public AbstractCallback
{
public:
    ServiceCallback( ServiceT* service );

    std::string     name() const override;
    const RequestT& request() const;
    ReplyT&         reply();

protected:
    virtual std::string methodType() const = 0;

protected:
    ServiceT* m_service;
    RequestT  m_request;
    ReplyT    m_reply;
};

/**
 * Templated callback for simple grpc services without streaming
 */
template <typename ServiceT, typename RequestT, typename ReplyT>
class UnaryCallback : public ServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using ResponseWriterT = grpc::ServerAsyncResponseWriter<ReplyT>;
    using MethodImplT     = std::function<grpc::Status( ServiceT&, grpc::ServerContext*, const RequestT*, ReplyT* )>;
    using MethodRequestT  = std::function<
        void( ServiceT&, grpc::ServerContext*, RequestT*, ResponseWriterT*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void* )>;

    UnaryCallback( ServiceT* service, MethodImplT methodImpl, MethodRequestT methodRequest );

    AbstractCallback* emptyClone() const override;
    void              createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) override;
    void              onProcessRequest() override;

protected:
    virtual std::string methodType() const override;

private:
    grpc::ServerContext m_context;
    ResponseWriterT     m_responder;
    MethodImplT         m_methodImpl;
    MethodRequestT      m_methodRequest;
};

/**
 * Templated abstract state handler for streaming callbacks
 * The implementations need a default constructor as well.
 */
template <typename RequestT>
class StateHandler
{
public:
    StateHandler()  = default;
    ~StateHandler() = default;

    virtual grpc::Status init( const RequestT* request ) = 0;
    virtual void         finish()                        = 0;

    virtual size_t streamedValueCount() const = 0;
    virtual size_t totalValueCount() const    = 0;

    virtual StateHandler<RequestT>* emptyClone() const = 0;
};

/**
 * A server to client streaming callback
 */
template <typename ServiceT, typename RequestT, typename ReplyT>
class ServerToClientStreamCallback : public ServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using ResponseWriterT = grpc::ServerAsyncWriter<ReplyT>;
    using MethodImplT =
        std::function<grpc::Status( ServiceT&, grpc::ServerContext*, const RequestT*, ReplyT*, StateHandler<RequestT>* )>;
    using MethodRequestT = std::function<
        void( ServiceT&, grpc::ServerContext*, RequestT*, ResponseWriterT*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void* )>;

    ServerToClientStreamCallback( ServiceT*               service,
                                  MethodImplT             methodImpl,
                                  MethodRequestT          methodRequest,
                                  StateHandler<RequestT>* stateHandler );

    AbstractCallback* emptyClone() const override;
    void              createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) override;
    void              onInitRequestCompleted() override;
    void              onProcessRequest() override;

protected:
    virtual std::string methodType() const override;

private:
    grpc::ServerContext                     m_context;
    ResponseWriterT                         m_responder;
    MethodImplT                             m_methodImpl;
    MethodRequestT                          m_methodRequest;
    std::unique_ptr<StateHandler<RequestT>> m_stateHandler;
};

/**
 * A client to server streaming callback
 */
template <typename ServiceT, typename RequestT, typename ReplyT>
class ClientToServerStreamCallback : public ServiceCallback<ServiceT, RequestT, ReplyT>
{
public:
    using RequestReaderT = grpc::ServerAsyncReader<ReplyT, RequestT>;
    using MethodImplT =
        std::function<grpc::Status( ServiceT&, grpc::ServerContext*, const RequestT*, ReplyT*, StateHandler<RequestT>* )>;
    using MethodRequestT = std::function<
        void( ServiceT&, grpc::ServerContext*, RequestReaderT*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void* )>;

    ClientToServerStreamCallback( ServiceT*               service,
                                  MethodImplT             methodImpl,
                                  MethodRequestT          methodRequest,
                                  StateHandler<RequestT>* stateHandler );

    AbstractCallback* emptyClone() const override;
    void              createRequestHandler( grpc::ServerCompletionQueue* completionQueue ) override;
    void              onInitRequestStarted() override;
    void              onInitRequestCompleted() override;
    void              onProcessRequest() override;

protected:
    virtual std::string methodType() const override;

private:
    grpc::ServerContext                     m_context;
    RequestReaderT                          m_reader;
    MethodImplT                             m_methodImpl;
    MethodRequestT                          m_methodRequest;
    std::unique_ptr<StateHandler<RequestT>> m_stateHandler;
};

} // namespace caffa::rpc
#include "cafGrpcCallbacks.inl"
