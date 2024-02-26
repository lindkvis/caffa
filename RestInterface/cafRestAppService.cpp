// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
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
#include "cafRestAppService.h"

#include "cafLogger.h"
#include "cafRestRequest.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"

#include <nlohmann/json.hpp>

using namespace caffa::rpc;

RestAppService::RestAppService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "app" );

    auto info = std::make_unique<RestPathEntry>( "info" );
    {
        auto action =
            std::make_unique<RestAction>( http::verb::get, "Get Application Information", "info", &RestAppService::info );
        action->addResponse( http::status::ok,
                             std::make_unique<RestResponse>( "application/json",
                                                             "#/components/app_schemas/AppInfo",
                                                             "Application Information" ) );
        action->addResponse( http::status::too_many_requests, RestResponse::plainError() );
        action->setRequiresAuthentication( false );
        action->setRequiresSession( false );

        info->addAction( std::move( action ) );
    }

    auto quit = std::make_unique<RestPathEntry>( "quit" );
    {
        auto action =
            std::make_unique<RestAction>( http::verb::delete_, "Quit Application", "quit", &RestAppService::quit );
        action->addResponse( http::status::accepted, std::make_unique<RestResponse>( "Success" ) );
        action->addResponse( http::status::forbidden, RestResponse::plainError() );
        action->setRequiresAuthentication( false );
        action->setRequiresSession( true );

        quit->addAction( std::move( action ) );
    }

    m_requestPathRoot->addEntry( std::move( info ) );
    m_requestPathRoot->addEntry( std::move( quit ) );
}

RestAppService::ServiceResponse RestAppService::perform( http::verb             verb,
                                                         std::list<std::string> path,
                                                         const nlohmann::json&  queryParams,
                                                         const nlohmann::json&  body )
{
    caffa::SessionMaintainer session;

    CAFFA_ASSERT( !path.empty() );

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    auto request = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_tuple( http::status::bad_request, "Path not found", nullptr );
    }

    return request->perform( verb, queryParams, body );
}

bool RestAppService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    auto service = m_requestPathRoot->findPathEntry( path );
    if ( !service ) return false;

    return service->requiresAuthentication( verb );
}

bool RestAppService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    auto service = m_requestPathRoot->findPathEntry( path );
    if ( !service ) return false;

    return service->requiresSession( verb );
}

std::map<std::string, nlohmann::json> RestAppService::servicePathEntries() const
{
    CAFFA_DEBUG( "Get service path entries" );

    auto services = nlohmann::json::object();

    RequestFinder finder( m_requestPathRoot.get() );
    finder.search();

    CAFFA_DEBUG( "Got " << finder.allPathEntriesWithActions().size() << " service path entries" );

    for ( const auto& [path, request] : finder.allPathEntriesWithActions() )
    {
        CAFFA_INFO( "Got path: " << path );
        services[path] = request->schema();
    }
    return services;
}

std::map<std::string, nlohmann::json> RestAppService::serviceComponentEntries() const
{
    auto appInfo = AppInfo::jsonSchema();

    return { { "app_schemas", { { "AppInfo", appInfo } } } };
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestAppService::ServiceResponse RestAppService::info( const nlohmann::json& queryParams, const nlohmann::json& body )
{
    CAFFA_TRACE( "Received info request" );

    auto app     = RestServerApplication::instance();
    auto appInfo = app->appInfo();

    nlohmann::json json = appInfo;
    return std::make_tuple( http::status::ok, json.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestAppService::ServiceResponse RestAppService::quit( const nlohmann::json& queryParams, const nlohmann::json& body )
{
    CAFFA_DEBUG( "Received quit request" );

    return std::make_tuple( http::status::accepted,
                            "Told to quit. It will happen soon",
                            []() { RestServerApplication::instance()->quit(); } );
}
