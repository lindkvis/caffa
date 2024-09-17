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
#include "cafStringTools.h"

using namespace caffa::rpc;
using namespace caffa;

RestAppService::RestAppService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "app" );

    auto info = std::make_unique<RestPathEntry>( "info" );
    {
        auto action =
            std::make_unique<RestAction>( http::verb::get, "Get Application Information", "info", &RestAppService::info );
        action->addResponse( http::status::ok,
                             RestResponse::objectResponse( "#/components/app_schemas/AppInfo", "Application Information" ) );
        action->addResponse( http::status::too_many_requests, RestResponse::plainErrorResponse() );
        action->setRequiresAuthentication( false );
        action->setRequiresSession( false );

        info->addAction( std::move( action ) );
    }

    m_requestPathRoot->addEntry( std::move( info ) );
}

RestAppService::ServiceResponse RestAppService::perform( const http::verb       verb,
                                                         std::list<std::string> path,
                                                         const json::object&    queryParams,
                                                         const json::value&     body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_pair( http::status::bad_request,
                               "App Path not found: " + caffa::StringTools::join( path.begin(), path.end(), "/" ) );
    }

    return request->perform( verb, pathArguments, queryParams, body );
}

bool RestAppService::requiresAuthentication( const http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request ) return false;

    return request->requiresAuthentication( verb );
}

bool RestAppService::requiresSession( const http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request ) return false;

    return request->requiresSession( verb );
}

std::map<std::string, json::object> RestAppService::servicePathEntries() const
{
    CAFFA_DEBUG( "Get service path entries" );

    std::map<std::string, json::object> services;

    RequestFinder finder( m_requestPathRoot.get() );
    finder.search();

    CAFFA_DEBUG( "Got " << finder.allPathEntriesWithActions().size() << " service path entries" );

    for ( const auto& [path, request] : finder.allPathEntriesWithActions() )
    {
        CAFFA_DEBUG( "Got path: " << path );
        services[path] = request->schema();
    }
    return services;
}

std::map<std::string, json::object> RestAppService::serviceComponentEntries() const
{
    auto appInfo = appJsonSchema();

    return { { "app_schemas", { { "AppInfo", appInfo } } } };
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestAppService::ServiceResponse RestAppService::info( http::verb                    verb,
                                                      const std::list<std::string>& pathArguments,
                                                      const json::object&           queryParams,
                                                      const json::value&            body )
{
    CAFFA_TRACE( "Received info request" );

    const auto app     = RestServerApplication::instance();
    auto       appInfo = app->appInfo();

    const json::value json = json::to_json( appInfo );
    return std::make_pair( http::status::ok, json::dump( json ) );
}
