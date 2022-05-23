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
#include "cafGrpcSession.h"
#include "cafLogger.h"

#include "AppInfo.pb.h"

using namespace caffa::rpc;

grpc::Status AppService::PerformQuit( grpc::ServerContext*, const NullMessage*, NullMessage* )
{
    CAFFA_DEBUG( "Received quit request" );
    ServerApplication::instance()->quit();
    return grpc::Status::OK;
}

grpc::Status AppService::PerformGetAppInfo( grpc::ServerContext*, const NullMessage*, AppInfoReply* reply )
{
    CAFFA_DEBUG( "Received app info request" );
    Application* app = Application::instance();
    reply->set_name( app->name() );
    CAFFA_ASSERT( app->hasCapability( AppInfo::AppCapability::GRPC_SERVER ) );

    AppInfo appInfo = app->appInfo();
    reply->set_type( appInfo.appType );
    reply->set_major_version( appInfo.majorVersion );
    reply->set_minor_version( appInfo.minorVersion );
    reply->set_patch_version( appInfo.patchVersion );

    return grpc::Status::OK;
}

grpc::Status AppService::PerformPing( grpc::ServerContext*, const NullMessage*, NullMessage* )
{
    CAFFA_DEBUG( "Received ping request" );
    return grpc::Status::OK;
}

grpc::Status AppService::PerformResetToDefaultData( grpc::ServerContext*, const NullMessage*, NullMessage* )
{
    CAFFA_DEBUG( "Received reset request" );
    ServerApplication::instance()->resetToDefaultData();

    return grpc::Status::OK;
}

grpc::Status AppService::CreateSession( grpc::ServerContext* context, const NullMessage* request, SessionMessage* reply )
{
    CAFFA_DEBUG( "Received create session request" );

    try
    {
        Session* session = ServerApplication::instance()->createSession();
        if ( !session ) throw std::runtime_error( "Failed to create session" );
        reply->set_uuid( session->uuid() );
        CAFFA_TRACE( "Created session: " << session->uuid() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return grpc::Status( grpc::FAILED_PRECONDITION, std::string( "Failed to create session with error: " ) + e.what() );
    }

    return grpc::Status::OK;
}

grpc::Status AppService::KeepSessionAlive( grpc::ServerContext* context, const SessionMessage* request, NullMessage* reply )
{
    CAFFA_TRACE( "Received session keep-alive from " << request->uuid() );
    try
    {
        ServerApplication::instance()->keepAliveSession( request->uuid() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to keep alive session with error: " << e.what() );
        return grpc::Status( grpc::FAILED_PRECONDITION,
                             std::string( "Failed to keep alive session with error: " ) + e.what() );
    }

    return grpc::Status::OK;
}

grpc::Status AppService::DestroySession( grpc::ServerContext* context, const SessionMessage* request, NullMessage* reply )
{
    CAFFA_DEBUG( "Received destroy session request for " << request->uuid() );

    try
    {
        ServerApplication::instance()->destroySession( request->uuid() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session did not exist. It may already have been destroyed due to lack of keepalive" );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> AppService::createCallbacks()
{
    typedef AppService Self;
    return {
        new UnaryCallback<Self, NullMessage, NullMessage>( this, &Self::PerformQuit, &Self::RequestQuit ),
        new UnaryCallback<Self, NullMessage, AppInfoReply>( this, &Self::PerformGetAppInfo, &Self::RequestGetAppInfo ),
        new UnaryCallback<Self, NullMessage, NullMessage>( this, &Self::PerformPing, &Self::RequestPing ),
        new UnaryCallback<Self, NullMessage, NullMessage>( this,
                                                           &Self::PerformResetToDefaultData,
                                                           &Self::RequestResetToDefaultData ),
        new UnaryCallback<Self, NullMessage, SessionMessage>( this, &Self::CreateSession, &Self::RequestCreateSession ),
        new UnaryCallback<Self, SessionMessage, NullMessage>( this, &Self::KeepSessionAlive, &Self::RequestKeepSessionAlive ),
        new UnaryCallback<Self, SessionMessage, NullMessage>( this, &Self::DestroySession, &Self::RequestDestroySession ),
    };
}
