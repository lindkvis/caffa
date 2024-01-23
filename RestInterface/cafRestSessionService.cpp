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

#include "cafLogger.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"

#include <nlohmann/json.hpp>

using namespace caffa::rpc;

RestSessionService::ServiceResponse RestSessionService::perform( http::verb             verb,
                                                                 std::list<std::string> path,
                                                                 const nlohmann::json&  queryParams,
                                                                 const nlohmann::json&  body )
{
    if ( path.empty() )
    {
        return performOnAll( verb, queryParams, body );
    }
    CAFFA_DEBUG( "Performing session request: " << path.front() );

    auto uuid = path.front();
    return performOnOne( verb, body, uuid );
}

bool RestSessionService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    // Create requests have no session and requires authentication
    bool isCreateRequest = path.empty() && verb == http::verb::post;
    return isCreateRequest;
}

bool RestSessionService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    // Create requests use authentication instead while the ready requests are completely unauthenticated
    bool isCreateRequest = path.empty() && verb == http::verb::post;
    bool isReadyRequest  = path.empty() && verb == http::verb::get;
    // Allow destruction of a session without a valid session uuid just so we can return not_found instead of forbidden
    bool isDestroyRequest = !path.empty() && verb == http::verb::delete_;
    return !( isCreateRequest || isReadyRequest || isDestroyRequest);
}

nlohmann::json RestSessionService::createOperation( const std::string&    operationId,
                                                    const std::string&    summary,
                                                    const nlohmann::json& parameters,
                                                    const nlohmann::json& responses,
                                                    const nlohmann::json& requestBody )
{
    auto tags = nlohmann::json::array( { "sessions" } );
    return RestServiceInterface::createOperation( operationId, summary, parameters, responses, requestBody, tags );
}

std::map<std::string, nlohmann::json> RestSessionService::servicePathEntries() const
{
    auto sessionObject                = nlohmann::json::object();
    sessionObject["application/json"] = { { "schema", { { "$ref", "#/components/session_schemas/Session" } } } };
    auto sessionContent = nlohmann::json{ { "description", "An application session" }, { "content", sessionObject } };

    auto createGetPutResponses =
        nlohmann::json{ { HTTP_OK, sessionContent }, { "default", RestServiceInterface::plainErrorResponse() } };

    auto emptyResponseContent = nlohmann::json{ { "description", "Success" } };

    auto acceptedOrFailureResponses = nlohmann::json{ { HTTP_ACCEPTED, emptyResponseContent },
                                                      { "default", RestServiceInterface::plainErrorResponse() } };

    auto sessions = nlohmann::json::object();

    auto typeParameter = nlohmann::json{ { "name", "type" },
                                         { "in", "path" },
                                         { "required", false },
                                         { "default", static_cast<unsigned>( caffa::Session::Type::REGULAR ) },
                                         { "description", "The type of session to query for" },
                                         { "schema", { { "type", "integer" }, { "format", "int32" } } } };

    auto readyValue = nlohmann::json{
        { "application/json", { { "schema", { { "$ref", "#/components/session_schemas/ReadyState" } } } } } };
    auto readyContent = nlohmann::json{ { "description", "Ready State" }, { "content", readyValue } };

    auto readyResponses =
        nlohmann::json{ { HTTP_OK, readyContent }, { "default", RestServiceInterface::plainErrorResponse() } };

    sessions["get"] =
        createOperation( "readyForSession", "Check if app is ready for session", typeParameter, readyResponses );

    sessions["post"] = createOperation( "createSession", "Create a new session", typeParameter, createGetPutResponses );

    auto uuidParameter = nlohmann::json{ { "name", "uuid" },
                                         { "in", "query" },
                                         { "required", true },
                                         { "description", "The session UUID of the session to get" },
                                         { "schema", { { "type", "string" } } } };

    auto session   = nlohmann::json::object();
    session["get"] = createOperation( "getSession", "Get a particular session", uuidParameter, createGetPutResponses );

    session["delete"] =
        createOperation( "destroySession", "Destroy a particular session", uuidParameter, acceptedOrFailureResponses );

    session["patch"] =
        createOperation( "keepSessionAlive", "Keep a particular session alive", uuidParameter, acceptedOrFailureResponses );

    session["put"] =
        createOperation( "changeSession", "Change a session", uuidParameter, createGetPutResponses, sessionContent );

    return { { "/sessions", sessions }, { "/sessions/{uuid}", session } };
}

std::map<std::string, nlohmann::json> RestSessionService::serviceComponentEntries() const
{
    auto session = nlohmann::json{ { "type", "object" },
                                   { "properties",
                                     { { "uuid", { { "type", "string" } } },
                                       { "type", { { "type", "integer" }, { "format", "int32" } } },
                                       { "valid", { { "type", "boolean" } } } } } };

    auto ready = nlohmann::json{ { "type", "object" },
                                 { "properties",
                                   {
                                       { "ready", { { "type", "boolean" } } },
                                       { "other_sessions", { { "type", "boolean" } } },
                                   } } };

    return { { "session_schemas", { { "Session", session }, { "ReadyState", ready } } } };
}

RestSessionService::ServiceResponse RestSessionService::performOnAll( http::verb verb, const nlohmann::json& queryParams, const nlohmann::json& body )
{
    switch ( verb )
    {
        case http::verb::get:
            return ready( queryParams );
        case http::verb::post:
            return create( body );
        default:
            CAFFA_WARNING( "Invalid sessions request " << http::to_string(verb) );
    }
    return std::make_tuple( http::status::bad_request, "Invalid sessions requests", nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::ready( const nlohmann::json& body )
{
    CAFFA_DEBUG( "Received ready for session request with metadata " << body );
    try
    {
        caffa::Session::Type type = caffa::Session::Type::REGULAR;
        if ( body.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( body["type"].get<unsigned>() );
        }
        auto jsonResponse = nlohmann::json::object();
        CAFFA_DEBUG("Checking if we're ready for a session of type " << static_cast<unsigned>(type));

        bool ready = RestServerApplication::instance()->readyForSession( type );

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
RestSessionService::ServiceResponse RestSessionService::create( const nlohmann::json& body )
{
    CAFFA_DEBUG( "Received create session request" );

    try
    {
        caffa::Session::Type type = caffa::Session::Type::REGULAR;
        if ( body.contains( "type" ) )
        {
            type = caffa::Session::typeFromUint( body["type"].get<unsigned>() );
        }
        auto session = RestServerApplication::instance()->createSession( type );

        CAFFA_TRACE( "Created session: " << session->uuid() );

        auto jsonResponse     = nlohmann::json::object();
        jsonResponse["uuid"]  = session->uuid();
        jsonResponse["type"]  = static_cast<unsigned>( session->type() );
        jsonResponse["valid"] = !session->isExpired();
        return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return std::make_tuple( http::status::forbidden, e.what(), nullptr );
    }
}

RestSessionService::ServiceResponse
    RestSessionService::performOnOne( http::verb verb, const nlohmann::json& body, const std::string& uuid )
{
    switch ( verb )
    {
        case http::verb::get:
            return get( uuid );
        case http::verb::put:
            return change( uuid, body );
        case http::verb::delete_:
            return destroy( uuid );
        case http::verb::patch:
            return keepalive( uuid );
        default:
            CAFFA_WARNING( "Invalid individual session request" );
    }
    return std::make_tuple( http::status::bad_request, "Invalid indidual session request", nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::get( const std::string& uuid )
{
    CAFFA_TRACE( "Got session get request for uuid " << uuid );

    caffa::SessionMaintainer session = RestServerApplication::instance()->getExistingSession( uuid );
    if ( !session )
    {
        return std::make_tuple( http::status::not_found, "Session '" + uuid + "' is not valid", nullptr );
    }

    auto jsonResponse     = nlohmann::json::object();
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = static_cast<unsigned>( session->type() );
    jsonResponse["valid"] = !session->isExpired();

    return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::change( const std::string& uuid, const nlohmann::json& body )
{
    CAFFA_TRACE( "Got session change request for " << uuid );

    caffa::SessionMaintainer session = RestServerApplication::instance()->getExistingSession( uuid );

    if ( !session )
    {
        return std::make_tuple( http::status::not_found, "Session '" + uuid + "' is not valid", nullptr );
    }
    else if ( session->isExpired() )
    {
        return std::make_tuple( http::status::gone, "Session '" + uuid + "' is expired", nullptr );
    }

    if ( !body.contains( "type" ) )
    {
        return std::make_tuple( http::status::bad_request, "No new type provided", nullptr );
    }

    caffa::Session::Type type = caffa::Session::typeFromUint( body["type"].get<unsigned>() );

    RestServerApplication::instance()->changeSession( session.get(), type );

    auto jsonResponse     = nlohmann::json::object();
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = static_cast<unsigned>( session->type() );
    jsonResponse["valid"] = !session->isExpired();

    return std::make_tuple( http::status::ok, jsonResponse.dump(), nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::keepalive( const std::string& uuid )
{
    CAFFA_TRACE( "Got session keep-alive request for " << uuid );

    auto session = RestServerApplication::instance()->getExistingSession( uuid );
    if ( !session )
    {
        return std::make_tuple( http::status::not_found, "Session '" + uuid + "' is not valid", nullptr );
    }
    else if ( session->isExpired() )
    {
        return std::make_tuple( http::status::gone, "Session '" + uuid + "' is expired", nullptr );
    }

    session->updateKeepAlive();
    return std::make_tuple( http::status::accepted, "", nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::destroy( const std::string& uuid )
{
    CAFFA_DEBUG( "Got destroy session request for " << uuid );

    try
    {
        RestServerApplication::instance()->destroySession( uuid );
        return std::make_tuple( http::status::accepted, "Session successfully destroyed", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session '" << uuid
                                   << "' did not exist. It may already have been destroyed due to lack of keepalive" );
        return std::make_tuple( http::status::not_found,
                                "Failed to destroy session. It may already have been destroyed.",
                                nullptr );
    }
}
