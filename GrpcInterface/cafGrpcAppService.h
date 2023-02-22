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

#include "cafGrpcServiceInterface.h"

#include <google/protobuf/empty.pb.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>

#include "App.grpc.pb.h"

#include <vector>

namespace caffa::rpc
{
class AbstractCallback;
class Version;

/**
 * Implementation of the App.proto-service
 */
class AppService : public ServiceInterface, public App::AsyncService
{
public:
    grpc::Status PerformQuit( grpc::ServerContext* context, const SessionMessage* request, NullMessage* reply );
    grpc::Status PerformGetAppInfo( grpc::ServerContext* context, const NullMessage* request, AppInfoReply* reply );
    grpc::Status PerformPing( grpc::ServerContext* context, const NullMessage* request, NullMessage* reply );
    grpc::Status
        ReadyForSession( grpc::ServerContext* context, const SessionParameters* request, NullMessage* reply ) override;
    grpc::Status
        CreateSession( grpc::ServerContext* context, const SessionParameters* request, SessionMessage* reply ) override;
    grpc::Status
        KeepSessionAlive( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply ) override;
    grpc::Status CheckSession( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply ) override;
    grpc::Status ChangeSession( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply ) override;
    grpc::Status DestroySession( grpc::ServerContext* context, const SessionMessage* request, NullMessage* reply ) override;

    std::vector<AbstractCallback*> createCallbacks() override;
};
} // namespace caffa::rpc
