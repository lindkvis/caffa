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
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafRestObjectService.h"
#include "cafRestServerApplication.h"
#include "cafSession.h"
#include "cafUuidGenerator.h"

using namespace caffa::rpc;
using namespace caffa;

RestSessionService::RestSessionService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "sessions" );

    auto sessionResponse = RestResponse::objectResponse( "#/components/session_schemas/Session", "An application session" );

    auto typeParameter = std::make_unique<RestTypedParameter<AppEnum<Session::Type>>>( "type",
                                                                                       RestParameter::Location::QUERY,
                                                                                       false,
                                                                                       "The type of session" );
    typeParameter->setDefaultValue( Session::Type::REGULAR );

    {
        auto readyAction = std::make_unique<RestAction>( http::verb::get,
                                                         "Check if App is ready for a new session",
                                                         "ready",
                                                         &RestSessionService::ready );

        readyAction->addParameter( typeParameter->clone() );
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
        uuidEntry->setPathArgumentMatcher( &UuidGenerator::isUuid );

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

        auto optionsAction = std::make_unique<RestAction>( http::verb::options,
                                                           "Get metadata for a particular session",
                                                           "getMetadata",
                                                           &RestSessionService::metadata );

        optionsAction->addParameter( uuidParameter->clone() );
        optionsAction->addResponse( http::status::ok,
                                    std::make_unique<RestResponse>( RestObjectService::anyObjectResponseContent(),
                                                                    "Any object" ) );

        optionsAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        optionsAction->setRequiresAuthentication( false );
        optionsAction->setRequiresSession( false );

        uuidEntry->addAction( std::move( optionsAction ) );

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
                                                                 const json::object&    queryParams,
                                                                 const json::value&     body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_pair( http::status::bad_request,
                               "Session Path not found: " + StringTools::join( path.begin(), path.end(), "/" ) );
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

std::map<std::string, json::object> RestSessionService::servicePathEntries() const
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

std::map<std::string, json::object> RestSessionService::serviceComponentEntries() const
{
    auto        sessionTypeLabels = AppEnum<Session::Type>::validLabels();
    json::array jsonTypeLabels;
    for ( auto label : sessionTypeLabels )
    {
        jsonTypeLabels.push_back( json::to_json( label ) );
    }

    json::object session = { { "type", "object" },
                             { "properties",
                               { { "uuid", { { "type", "string" } } },
                                 { "type", { { "type", "string" }, { "enum", jsonTypeLabels } } },
                                 { "valid", { { "type", "boolean" } } } } } };

    json::object ready = { { "type", "object" },
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
                                                               const json::object&           queryParams,
                                                               const json::value&            body )
{
    CAFFA_DEBUG( "Received ready for session request with metadata " << queryParams );
    try
    {
        AppEnum<Session::Type> type;
        if ( const auto it = queryParams.find( "type" ); it != queryParams.end() )
        {
            type.setFromLabel( json::from_json<std::string>( it->value() ) );
        }
        json::object jsonResponse;

        const bool ready = RestServerApplication::instance()->readyForSession( type.value() );

        jsonResponse["ready"]          = ready;
        jsonResponse["other_sessions"] = RestServerApplication::instance()->hasActiveSessions();
        CAFFA_DEBUG( "Checking if we're ready for a session of type " << type.label() << "? " << ready);
        return std::make_pair( http::status::ok, json::dump( jsonResponse ) );
    }
    catch ( ... )
    {
        return std::make_pair( http::status::not_found, "Failed to check for session readiness" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::create( http::verb,
                                                                const std::list<std::string>&,
                                                                const json::object& queryParams,
                                                                const json::value& )
{
    CAFFA_DEBUG( "Received create session request" );

    try
    {
        AppEnum<Session::Type> type;

        if ( const auto it = queryParams.find( "type" ); it != queryParams.end() )
        {
            type.setFromLabel( json::from_json<std::string>( it->value() ) );
        }
        auto session = RestServerApplication::instance()->createSession( type.value() );

        CAFFA_TRACE( "Created session: " << session->uuid() );

        json::object jsonResponse;
        jsonResponse["uuid"]  = session->uuid();
        jsonResponse["type"]  = AppEnum<Session::Type>::getLabel( session->type() );
        jsonResponse["valid"] = RestServerApplication::instance()->isValid( session.get() );
        return std::make_pair( http::status::ok, json::dump( jsonResponse ) );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to create session with error: " << e.what() );
        return std::make_pair( http::status::forbidden, e.what() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::get( http::verb,
                                                             const std::list<std::string>& pathArguments,
                                                             const json::object&,
                                                             const json::value& )
{
    if ( pathArguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Session uuid not provided" );
    }
    const auto& uuid = pathArguments.front();

    CAFFA_DEBUG( "Got session get request for uuid " << uuid );

    const auto session = RestServerApplication::instance()->getExistingSession( uuid );
    if ( !session )
    {
        if ( !RestServerApplication::instance()->wasValidSession( uuid ) )
        {
            return std::make_pair( http::status::not_found, "Session '" + uuid + "' is not valid" );
        }
        json::object jsonResponse;
        jsonResponse["uuid"]  = uuid;
        jsonResponse["type"]  = AppEnum<Session::Type>::getLabel( Session::Type::UNKNOWN );
        jsonResponse["valid"] = false;

        return std::make_pair( http::status::ok, json::dump( jsonResponse ) );
    }

    json::object jsonResponse;
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = AppEnum<Session::Type>::getLabel( session->type() );
    jsonResponse["valid"] = RestServerApplication::instance()->isValid( session.get() );

    return std::make_pair( http::status::ok, json::dump( jsonResponse ) );
}
RestServiceInterface::ServiceResponse RestSessionService::metadata( http::verb                    verb,
                                                                    const std::list<std::string>& pathArguments,
                                                                    const json::object&           queryParams,
                                                                    const json::value&            body )
{
    if ( pathArguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Session uuid not provided" );
    }
    const auto& uuid = pathArguments.front();

    CAFFA_DEBUG( "Got session metadata request for uuid " << uuid );

    const std::shared_ptr<ObjectHandle> metadata = RestServerApplication::instance()->sessionMetadata( uuid );
    if ( !metadata )
    {
        return std::make_pair( http::status::not_found, "Session '" + uuid + "' metadata not found" );
    }

    return std::make_pair( http::status::ok, JsonSerializer().writeObjectToString( metadata.get() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::changeOrKeepAlive( http::verb,
                                                                           const std::list<std::string>& pathArguments,
                                                                           const json::object&           queryParams,
                                                                           const json::value& )
{
    if ( pathArguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Session uuid not provided" );
    }
    const auto& uuid = pathArguments.front();

    CAFFA_TRACE( "Got session change/keepalive request for " << uuid );

    const std::shared_ptr<Session> session = RestServerApplication::instance()->getExistingSession( uuid );

    if ( !session )
    {
        return std::make_pair( http::status::not_found, "Session '" + uuid + "' is not valid" );
    }

    if ( !RestServerApplication::instance()->isValid( session.get() ) )
    {
        return std::make_pair( http::status::gone, "Session '" + uuid + "' is expired" );
    }

    if ( !queryParams.contains( "type" ) )
    {
        session->updateKeepAlive();
    }
    else
    {
        AppEnum<Session::Type> type;

        if ( const auto it = queryParams.find( "type" ); it != queryParams.end() )
        {
            type.setFromLabel( json::from_json<std::string>( it->value() ) );
        }

        RestServerApplication::instance()->changeSession( session.get(), type.value() );
    }
    auto jsonResponse     = json::object();
    jsonResponse["uuid"]  = session->uuid();
    jsonResponse["type"]  = AppEnum<Session::Type>::getLabel( session->type() );
    jsonResponse["valid"] = RestServerApplication::instance()->isValid( session.get() );

    return std::make_pair( http::status::ok, json::dump( jsonResponse ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestSessionService::ServiceResponse RestSessionService::destroy( http::verb,
                                                                 const std::list<std::string>& pathArguments,
                                                                 const json::object&,
                                                                 const json::value& )
{
    if ( pathArguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Session uuid not provided" );
    }
    const auto& uuid = pathArguments.front();

    CAFFA_DEBUG( "Got destroy session request for " << uuid );

    try
    {
        RestServerApplication::instance()->destroySession( uuid );
        return std::make_pair( http::status::accepted, "Session successfully destroyed" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_WARNING( "Session '" << uuid << "' did not exist. It may already have been destroyed due to lack of keepalive: "
                                   << e.what() );
        return std::make_pair( http::status::not_found, "Failed to destroy session. It may already have been destroyed." );
    }
}
