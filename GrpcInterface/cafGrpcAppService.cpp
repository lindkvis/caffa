// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
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
#include "cafGrpcAppService.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcServer.h"
#include "cafGrpcServerApplication.h"
#include "cafLogger.h"
#include "cafSession.h"

#include "App.pb.h"

using namespace caffa::rpc;

grpc::Status GrpcAppService::PerformQuit( grpc::ServerContext* context, const SessionMessage* request, NullMessage* )
{
    CAFFA_DEBUG( "Received quit request from " << context->peer() );

    auto session = GrpcServerApplication::instance()->getExistingSession( request->uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->uuid() + "' is not valid" );
    }

    GrpcServerApplication::instance()->quit();
    return grpc::Status::OK;
}

grpc::Status GrpcAppService::PerformGetAppInfo( grpc::ServerContext* context, const NullMessage*, AppInfoReply* reply )
{
    CAFFA_TRACE( "Received app info request from " + context->peer() );
    Application* app = Application::instance();
    reply->set_name( app->name() );
    CAFFA_ASSERT( app->hasCapability( AppInfo::AppCapability::SERVER ) );

    AppInfo appInfo = app->appInfo();
    reply->set_type( appInfo.appType );
    reply->set_major_version( appInfo.majorVersion );
    reply->set_minor_version( appInfo.minorVersion );
    reply->set_patch_version( appInfo.patchVersion );

    return grpc::Status::OK;
}

grpc::Status GrpcAppService::PerformPing( grpc::ServerContext* context, const NullMessage*, NullMessage* )
{
    CAFFA_TRACE( "Received ping request from " + context->peer() );
    return grpc::Status::OK;
}

grpc::Status
    GrpcAppService::ReadyForSession( grpc::ServerContext* context, const SessionParameters* request, ReadyMessage* reply )
{
    CAFFA_TRACE( "Received ready for session request from " + context->peer() );

    bool ready = GrpcServerApplication::instance()->readyForSession( caffa::Session::typeFromUint( request->type() ) );
    reply->set_ready( ready );
    reply->set_has_other_sessions( GrpcServerApplication::instance()->hasActiveSessions() );

    return grpc::Status::OK;
}

grpc::Status
    GrpcAppService::CreateSession( grpc::ServerContext* context, const SessionParameters* request, SessionMessage* reply )
{
    CAFFA_DEBUG( "Received create session request from " << context->peer() );

    try
    {
        auto session = GrpcServerApplication::instance()->createSession( caffa::Session::typeFromUint( request->type() ) );
        if ( !session )
        {
            throw std::runtime_error( "Failed to create session" );
        }
        reply->set_uuid( session->uuid() );
        reply->set_type( static_cast<caffa::rpc::SessionType>( session->type() ) );
        CAFFA_TRACE( "Created session: " << session->uuid() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return grpc::Status( grpc::UNAUTHENTICATED, e.what() );
    }

    return grpc::Status::OK;
}

grpc::Status
    GrpcAppService::KeepSessionAlive( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply )
{
    CAFFA_TRACE( "Received session keep-alive from " << request->uuid() << " from " << context->peer() );
    auto session = GrpcServerApplication::instance()->getExistingSession( request->uuid() );
    if ( session )
    {
        try
        {
            session->updateKeepAlive();
            reply->set_uuid( request->uuid() );
            reply->set_type( static_cast<caffa::rpc::SessionType>( session->type() ) );
        }
        catch ( const std::exception& e )
        {
            CAFFA_ERROR( "Failed to keep alive session with error: " << e.what() );
            return grpc::Status( grpc::FAILED_PRECONDITION,
                                 std::string( "Failed to keep alive session with error: " ) + e.what() );
        }

        return grpc::Status::OK;
    }
    return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->uuid() + "' does not exist or has expired!" );
}

grpc::Status GrpcAppService::CheckSession( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply )
{
    CAFFA_TRACE( "Received session check from " << request->uuid() << " from " << context->peer() );
    auto session = GrpcServerApplication::instance()->getExistingSession( request->uuid() );
    if ( session )
    {
        CAFFA_ASSERT( session->uuid() == request->uuid() );
        reply->set_uuid( request->uuid() );
        reply->set_type( static_cast<caffa::rpc::SessionType>( session->type() ) );

        return grpc::Status::OK;
    }
    return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->uuid() + "' does not exist or has expired!" );
}

grpc::Status
    GrpcAppService::ChangeSession( grpc::ServerContext* context, const SessionMessage* request, SessionMessage* reply )
{
    CAFFA_TRACE( "Received session check from " << request->uuid() << " from " << context->peer() );
    auto session = GrpcServerApplication::instance()->getExistingSession( request->uuid() );
    if ( session )
    {
        CAFFA_ASSERT( session->uuid() == request->uuid() );
        try
        {
            GrpcServerApplication::instance()->changeSession( session.get(),
                                                              caffa::Session::typeFromUint( request->type() ) );
            reply->set_uuid( request->uuid() );
            reply->set_type( static_cast<caffa::rpc::SessionType>( session->type() ) );
            return grpc::Status::OK;
        }
        catch ( const std::exception& e )
        {
            return grpc::Status( grpc::FAILED_PRECONDITION,
                                 std::string( "Failed to change session with error " ) + e.what() );
        }
    }
    return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->uuid() + "' does not exist or has expired!" );
}

grpc::Status GrpcAppService::DestroySession( grpc::ServerContext* context, const SessionMessage* request, NullMessage* reply )
{
    CAFFA_DEBUG( "Received destroy session request for " << request->uuid() << " from " << context->peer() );

    try
    {
        GrpcServerApplication::instance()->destroySession( request->uuid() );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session '" << request->uuid()
                                   << "' did not exist. It may already have been destroyed due to lack of keepalive" );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractGrpcCallback*> GrpcAppService::createCallbacks()
{
    typedef GrpcAppService Self;
    return {
        new GrpcServiceCallback<Self, SessionMessage, NullMessage>( this, &Self::PerformQuit, &Self::RequestQuit ),
        new GrpcServiceCallback<Self, NullMessage, AppInfoReply>( this, &Self::PerformGetAppInfo, &Self::RequestGetAppInfo ),
        new GrpcServiceCallback<Self, NullMessage, NullMessage>( this, &Self::PerformPing, &Self::RequestPing ),
        new GrpcServiceCallback<Self, SessionParameters, ReadyMessage>( this,
                                                                        &Self::ReadyForSession,
                                                                        &Self::RequestReadyForSession ),
        new GrpcServiceCallback<Self, SessionParameters, SessionMessage>( this,
                                                                          &Self::CreateSession,
                                                                          &Self::RequestCreateSession ),
        new GrpcServiceCallback<Self, SessionMessage, SessionMessage>( this,
                                                                       &Self::KeepSessionAlive,
                                                                       &Self::RequestKeepSessionAlive ),
        new GrpcServiceCallback<Self, SessionMessage, SessionMessage>( this, &Self::CheckSession, &Self::RequestCheckSession ),
        new GrpcServiceCallback<Self, SessionMessage, SessionMessage>( this, &Self::ChangeSession, &Self::RequestChangeSession ),
        new GrpcServiceCallback<Self, SessionMessage, NullMessage>( this, &Self::DestroySession, &Self::RequestDestroySession ),
    };
}
