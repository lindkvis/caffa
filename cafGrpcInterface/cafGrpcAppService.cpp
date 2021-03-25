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
#include "cafGrpcAppService.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcServer.h"
#include "cafGrpcServerApplication.h"

#include <grpcpp/impl/codegen/method_handler.h>
#include "AppInfo_generated.h"

using namespace caf::rpc;

grpc::Status AppService::Quit( grpc::ServerContext* context, const NullMessageT* request, NullMessageT* reply )
{
    ServerApplication::instance()->quit();
    return grpc::Status::OK;
}

grpc::Status AppService::GetAppInfo( grpc::ServerContext* context, const NullMessageT* request, AppInfoReplyT* reply )
{
    Application* app = Application::instance();
    CAF_ASSERT( app->hasCapability( AppCapability::GRPC_SERVER ) );
    AppInfo appInfo = app->appInfo();

    flatbuffers::grpc::MessageBuilder mb;

    auto appName = mb.CreateString(app->name());
    auto reply_offset = CreateAppInfoReply(mb, appName, appInfo.appType, appInfo.majorVersion, appInfo.minorVersion, appInfo.patchVersion);

    *reply = mb.ReleaseMessage<AppInfoReply>();
    CAF_ASSERT(reply->Verify());

    return grpc::Status::OK;
}

grpc::Status AppService::Ping(grpc::ServerContext* context, const NullMessageT* request, NullMessageT* reply) 
{
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> AppService::registerCallbacks()
{
    typedef AppService Self;
    return { new UnaryCallback<Self, NullMessageT, NullMessageT>( this, &Self::Quit),
             new UnaryCallback<Self, NullMessageT, AppInfoReplyT>( this, &Self::GetAppInfo),
             new UnaryCallback<Self, NullMessageT, NullMessageT>( this, &Self::Ping) };
}
