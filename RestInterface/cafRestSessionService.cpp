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
#include "cafRestSessionService.h"

#include "cafAppEnum.h"
#include "cafLogger.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"
#include "cafUuidGenerator.h"

#include <nlohmann/json.hpp>

using namespace caffa::rpc;

RestSessionService::RestSessionService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "sessions" );

    auto sessionResponse = RestResponse::objectResponse( "#/components/session_schemas/Session", "An application session" );

    auto typeParameter =
        std::make_unique<RestTypedParameter<caffa::AppEnum<caffa::Session::Type>>>( "type",
                                                                                    RestParameter::Location::QUERY,
                                                                                    false,
                                                                                    "The type of session" );
    typeParameter->setDefaultValue( caffa::Session::Type::REGULAR );

    {
        auto readyAction = std::make_unique<RestAction>( http::verb::get,
                                                         "Check if App is ready for a new session",
                                                         "ready",
                                                         &RestSessionService::ready );

        readyAction->addResponse( http::status::ok,
                                  RestResponse::objectResponse( "#/components/session_schemas/ReadyState", "Ready State" ) );

        readyAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        readyAction->setRequiresAuthentication( false );
        readyAction->setRequiresSession( false );

        auto createAction =
            std::make_unique<RestAction>( http::verb::post, "Create a new session", "create", &RestSessionService::create );

        createAction->addParameter( typeParameter->clone() );

        createAction->addResponse( http::status::accepted, sessionResponse->clone() );
        createAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        createAction->setRequiresAuthentication( true );
        createAction->setRequiresSession( false );

        m_requestPathRoot->addAction( std::move( readyAction ) );
        m_requestPathRoot->addAction( std::move( createAction ) );
    }

    {
        auto uuidEntry = std::make_unique<RestPathEntry>( "{uuid}" );
        uuidEntry->setPathArgumentMatcher( &caffa::UuidGenerator::isUuid );

        auto uuidParameter = std::make_unique<RestTypedParameter<std::string>>( "uuid",
                                                                                RestParameter::Location::PATH,
                                                                                true,
                                                                                "The UUID of the session to check " );

        auto getAction = std::make_unique<RestAction>( http::verb::get,
                                                       "Get status of a particular session",
                                                       "getSession",
                                                       &RestSessionService::get );

        getAction->addParameter( uuidParameter->clone() );
        getAction->addResponse( http::status::ok, sessionResponse->clone() );
        getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        getAction->setRequiresAuthentication( false );
        getAction->setRequiresSession( false );

        uuidEntry->addAction( std::move( getAction ) );

        auto deleteAction = std::make_unique<RestAction>( http::verb::delete_,
                                                          "Destroy a particular session",
                                                          "destroySession",
                                                          &RestSessionService::destroy );

        deleteAction->addParameter( uuidParameter->clone() );
        deleteAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        deleteAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        deleteAction->setRequiresAuthentication( false );
        deleteAction->setRequiresSession( false );
        uuidEntry->addAction( std::move( deleteAction ) );

        auto putAction = std::make_unique<RestAction>( http::verb::put,
                                                       "Change or keep session alive",
                                                       "changeOrKeepAliveSession",
                                                       &RestSessionService::changeOrKeepAlive );

        putAction->addParameter( uuidParameter->clone() );
        putAction->addParameter( typeParameter->clone() );

        putAction->addResponse( http::status::ok, sessionResponse->clone() );
        putAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        putAction->setRequiresAuthentication( false );
        putAction->setRequiresSession( false );
        uuidEntry->addAction( std::move( putAction ) );

        m_requestPathRoot->addEntry( std::move( uuidEntry ) );
    }
}

RestSessionService::ServiceResponse RestSessionService::perform( http::verb             verb,
                                                                 std::list<std::string> path,
                                                                 const nlohmann::json&  queryParams,
                                                                 const nlohmann::json&  body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_tuple( http::status::bad_request, "Path not found", nullptr );
    }

    return request->perform( verb, pathArguments, queryParams, body );
}

bool RestSessionService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }
    return request->requiresAuthentication( verb );
}

bool RestSessionService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }

    return request->requiresSession( verb );
}

std::map<std::string, nlohmann::json> RestSessionService::servicePathEntries() const
{
    CAFFA_DEBUG( "Get service path entries" );

    auto services = nlohmann::json::object();

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

std::map<std::string, nlohmann::json> RestSessionService::serviceComponentEntries() const
{
    auto sessionTypeLabels = caffa::AppEnum<caffa::Session::Type>::labels();
    auto jsonTypeLabels    = nlohmann::json::array();
    for ( auto label : sessionTypeLabels )
    {
        jsonTypeLabels.push_back( label );
    }

    auto session = nlohmann::json{ { "type", "object" },
                                   { "properties",
                                     { { "uuid", { { "type", "string" } } },
                                       { "type", { { "type", "string" }, { "enum", jsonTypeLabels } } },
                                       { "valid", { { "type", "boolean" } } } } } };

    auto ready = nlohmann::json{ { "type", "object" },
                                 { "properties",
                                   {
                                       { "ready", { { "type", "boolean" } } },
                                       { "other_sessions", { { "type", "boolean" } } },
                                   } } };

    return { { "session_schemas", { { "Session", session }, { "ReadyState", ready } } } };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::ready( http::verb                    verb,
                                                               const std::list<std::string>& pathArguments,
                                                               const nlohmann::json&         queryParams,
                                                               const nlohmann::json&         body )
{
    CAFFA_DEBUG( "Received ready for session request with metadata " << queryParams );
    try
    {
        caffa::AppEnum<caffa::Session::Type> type;
        if ( queryParams.contains( "type" ) )
        {
            type.setFromLabel( queryParams["type"].get<std::string>() );
        }
        auto jsonResponse = nlohmann::json::object();
        CAFFA_DEBUG( "Checking if we're ready for a session of type " << type.label() );

        bool ready = RestServerApplication::instance()->readyForSession( type.value() );

        jsonResponse["ready"]          = ready;
        jsonResponse["other_sessions"] = RestServerApplication::instance()->hasActiveSessions();
        return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
    }
    catch ( ... )
    {
        return std::make_tuple( http::status::not_found, "Failed to check for session readiness", nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::create( http::verb                    verb,
                                                                const std::list<std::string>& pathArguments,
                                                                const nlohmann::json&         queryParams,
                                                                const nlohmann::json&         body )
{
    CAFFA_DEBUG( "Received create session request" );

    try
    {
        caffa::AppEnum<caffa::Session::Type> type;
        if ( queryParams.contains( "type" ) )
        {
            type.setFromLabel( queryParams["type"].get<std::string>() );
        }
        auto session = RestServerApplication::instance()->createSession( type.value() );

        CAFFA_TRACE( "Created session: " << session->uuid() );

        auto jsonResponse     = nlohmann::json::object();
        jsonResponse["uuid"]  = session->uuid();
        jsonResponse["type"]  = caffa::AppEnum<caffa::Session::Type>::label( session->type() );
        jsonResponse["valid"] = !session->isExpired();
        return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return std::make_tuple( http::status::forbidden, e.what(), nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::get( http::verb                    verb,
                                                             const std::list<std::string>& pathArguments,
                                                             const nlohmann::json&         queryParams,
                                                             const nlohmann::json&         body )
{
    if ( pathArguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "Session uuid not provided", nullptr );
    }
    auto uuid = pathArguments.front();

    CAFFA_DEBUG( "Got session get request for uuid " << uuid );

    caffa::SessionMaintainer session = RestServerApplication::instance()->getExistingSession( uuid );
    if ( !session )
    {
        return std::make_tuple( http::status::not_found, "Session '" + uuid + "' is not valid", nullptr );
    }

    auto jsonResponse     = nlohmann::json::object();
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = caffa::AppEnum<caffa::Session::Type>::label( session->type() );
    jsonResponse["valid"] = !session->isExpired();

    return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::changeOrKeepAlive( http::verb                    verb,
                                                                           const std::list<std::string>& pathArguments,
                                                                           const nlohmann::json&         queryParams,
                                                                           const nlohmann::json&         body )
{
    if ( pathArguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "Session uuid not provided", nullptr );
    }
    auto uuid = pathArguments.front();

    CAFFA_DEBUG( "Got session change request for " << uuid );

    caffa::SessionMaintainer session = RestServerApplication::instance()->getExistingSession( uuid );

    if ( !session )
    {
        return std::make_tuple( http::status::not_found, "Session '" + uuid + "' is not valid", nullptr );
    }
    else if ( session->isExpired() )
    {
        return std::make_tuple( http::status::gone, "Session '" + uuid + "' is expired", nullptr );
    }

    if ( !queryParams.contains( "type" ) )
    {
        session->updateKeepAlive();
    }
    else
    {
        caffa::AppEnum<caffa::Session::Type> type;
        type.setFromLabel( queryParams["type"].get<std::string>() );

        RestServerApplication::instance()->changeSession( session.get(), type.value() );
    }
    auto jsonResponse     = nlohmann::json::object();
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = caffa::AppEnum<caffa::Session::Type>::label( session->type() );
    jsonResponse["valid"] = !session->isExpired();

    return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::destroy( http::verb                    verb,
                                                                 const std::list<std::string>& pathArguments,
                                                                 const nlohmann::json&         queryParams,
                                                                 const nlohmann::json&         body )
{
    if ( pathArguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "Session uuid not provided", nullptr );
    }
    auto uuid = pathArguments.front();

    CAFFA_DEBUG( "Got destroy session request for " << uuid );

    try
    {
        RestServerApplication::instance()->destroySession( uuid );
        return std::make_tuple( http::status::accepted, "Session successfully destroyed", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session '" << uuid << "' did not exist. It may already have been destroyed due to lack of keepalive: "
                                   << e.what() );
        return std::make_tuple( http::status::not_found,
                                "Failed to destroy session. It may already have been destroyed.",
                                nullptr );
    }
}
