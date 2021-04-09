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
#include "cafLogger.h"

#include "AppInfo.pb.h"

using namespace caf::rpc;

grpc::Status AppService::PerformQuit( grpc::ServerContext* context, const NullMessage* request, NullMessage* reply )
{
    CAF_DEBUG( "Received quit request" );
    ServerApplication::instance()->quit();
    return grpc::Status::OK;
}

grpc::Status AppService::PerformGetAppInfo( grpc::ServerContext* context, const NullMessage* request, AppInfoReply* reply )
{
    CAF_DEBUG( "Received app info request" );
    Application* app = Application::instance();
    reply->set_name( app->name() );
    CAF_ASSERT( app->hasCapability( AppCapability::GRPC_SERVER ) );

    AppInfo appInfo = app->appInfo();
    reply->set_type( appInfo.appType );
    reply->set_major_version( appInfo.majorVersion );
    reply->set_minor_version( appInfo.minorVersion );
    reply->set_patch_version( appInfo.patchVersion );

    return grpc::Status::OK;
}

grpc::Status AppService::PerformPing( grpc::ServerContext* context, const NullMessage* request, NullMessage* reply )
{
    CAF_DEBUG( "Received ping request" );
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> AppService::registerCallbacks()
{
    typedef AppService Self;
    return { new UnaryCallback<Self, NullMessage, NullMessage>( this, &Self::PerformQuit, &Self::RequestQuit ),
             new UnaryCallback<Self, NullMessage, AppInfoReply>( this, &Self::PerformGetAppInfo, &Self::RequestGetAppInfo ),
             new UnaryCallback<Self, NullMessage, NullMessage>( this, &Self::PerformPing, &Self::RequestPing ) };
}
