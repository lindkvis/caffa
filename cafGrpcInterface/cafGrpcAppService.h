//##################################################################################################
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
//
#pragma once

#include "cafGrpcServiceInterface.h"

#include "AppInfo.grpc.pb.h"

#include <google/protobuf/empty.pb.h>
#include <grpcpp/grpcpp.h>

#include <vector>

namespace caf::rpc
{
class AbstractCallback;
class Version;

/**
 * Implementation of the App.proto-service
 */
class AppService : public ServiceInterface, public App::AsyncService
{
public:
    grpc::Status Quit( grpc::ServerContext* context, const NullMessage* request, NullMessage* reply ) override;
    grpc::Status GetAppInfo( grpc::ServerContext* context, const NullMessage* request, AppInfoReply* reply ) override;
    grpc::Status Ping(grpc::ServerContext* context, const NullMessage* request, NullMessage* reply) override;

    std::vector<AbstractCallback*> registerCallbacks() override;
};
} // namespace caf::rpc
