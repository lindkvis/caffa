// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2024- Kontur AS
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
#include "cafRestRequest.h"

#include "cafLogger.h"
#include "cafStringTools.h"

using namespace caffa::rpc;

RestResponse::RestResponse( const nlohmann::json& contentSchema, const std::string& description )
    : m_content( contentSchema )
    , m_description( description )
{
}

nlohmann::json RestResponse::schema() const
{
    auto response = nlohmann::json{ { "description", m_description } };

    if ( !m_content.is_null() )
    {
        response["content"] = m_content;
    }

    return response;
}

std::unique_ptr<RestResponse> RestResponse::clone() const
{
    return std::make_unique<RestResponse>( m_content, m_description );
}

std::unique_ptr<RestResponse> RestResponse::emptyResponse( const std::string& description )
{
    return std::make_unique<RestResponse>( nlohmann::json(), description );
}

std::unique_ptr<RestResponse> RestResponse::plainErrorResponse()
{
    auto content          = nlohmann::json::object();
    content["text/plain"] = { { "schema", "#/components/error_schemas/PlainError" } };
    return std::make_unique<RestResponse>( content, "A Service Error Message" );
}

std::unique_ptr<RestResponse> RestResponse::objectResponse( const std::string& schemaPath, const std::string& description )
{
    auto content                = nlohmann::json::object();
    content["application/json"] = { { "schema", { { "$ref", schemaPath } } } };
    return std::make_unique<RestResponse>( content, description );
}

std::unique_ptr<RestResponse> RestResponse::objectArrayResponse( const std::string& schemaPath,
                                                                 const std::string& description )
{
    auto arraySchema    = nlohmann::json::object();
    arraySchema["type"] = "array";
    arraySchema["item"] = { { "$ref", schemaPath } };

    auto content                = nlohmann::json::object();
    content["application/json"] = arraySchema;
    return std::make_unique<RestResponse>( content, description );
}

RestParameter::RestParameter( const std::string& name, Location location, bool required, const std::string& description )
    : m_name( name )
    , m_location( location )
    , m_required( required )
    , m_description( description )
{
}

RestParameter::~RestParameter()
{
}

RestAction::RestAction( http::verb verb, const std::string& summary, const std::string& operationId, const Callback& callback )
    : m_verb( verb )
    , m_summary( summary )
    , m_operationId( operationId )
    , m_callback( callback )
    , m_requiresSession( true )
    , m_requiresAuthentication( true )
    , m_requestBodySchema( nullptr )
{
}

http::verb RestAction::verb() const
{
    return m_verb;
}

void RestAction::addTag( const std::string& tag )
{
    m_tags.push_back( tag );
}

void RestAction::addParameter( std::unique_ptr<RestParameter> parameter )
{
    m_parameters.push_back( std::move( parameter ) );
}

void RestAction::addResponse( http::status status, std::unique_ptr<RestResponse> response )
{
    CAFFA_ASSERT( !m_responses.contains( status ) );
    m_responses[status] = std::move( response );
}

nlohmann::json RestAction::schema() const
{
    auto tagList = nlohmann::json::array();
    for ( const auto& tag : m_tags )
    {
        tagList.push_back( tag );
    }

    auto responses = nlohmann::json::object();
    for ( const auto& [status, response] : m_responses )
    {
        std::string statusString = "default";
        if ( status != http::status::unknown )
        {
            statusString = std::to_string( static_cast<unsigned>( status ) );
        }
        responses[statusString] = response->schema();
    }

    auto action = nlohmann::json{ { "summary", m_summary },
                                  { "operationId", m_operationId },
                                  { "responses", responses },
                                  { "tags", tagList } };

    if ( !m_parameters.empty() )
    {
        auto parameters = nlohmann::json::array();
        for ( const auto& parameter : m_parameters )
        {
            parameters.push_back( parameter->schema() );
        }
        action["parameters"] = parameters;
    }

    if ( !m_requestBodySchema.is_null() )
    {
        action["requestBody"] = m_requestBodySchema;
    }
    return action;
}

RestAction::ServiceResponse RestAction::perform( const std::list<std::string>& pathArguments,
                                                 const nlohmann::json&         queryParams,
                                                 const nlohmann::json&         body ) const
{
    return m_callback( m_verb, pathArguments, queryParams, body );
}

void RestAction::setRequiresSession( bool requiresSession )
{
    m_requiresSession = requiresSession;
}

void RestAction::setRequiresAuthentication( bool requiresAuthentication )
{
    m_requiresAuthentication = requiresAuthentication;
}

bool RestAction::requiresSession() const
{
    return m_requiresSession;
}

bool RestAction::requiresAuthentication() const
{
    return m_requiresAuthentication;
}

void RestAction::setRequestBodySchema( const nlohmann::json& requestBodySchema )
{
    m_requestBodySchema = requestBodySchema;
}

RestPathEntry::RestPathEntry( const std::string& name )
    : m_name( name )
{
}

const std::string& RestPathEntry::name() const
{
    return m_name;
}

RestPathEntry::ServiceResponse RestPathEntry::perform( http::verb                    verb,
                                                       const std::list<std::string>& pathArguments,
                                                       const nlohmann::json&         queryParams,
                                                       const nlohmann::json&         body ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        return std::make_tuple( http::status::not_found,
                                "Could not find the verb " + std::string( http::to_string( verb ) ) + " in request " +
                                    name(),
                                nullptr );
    }

    return it->second->perform( pathArguments, queryParams, body );
}

void RestPathEntry::addEntry( std::unique_ptr<RestPathEntry> pathEntry )
{
    CAFFA_ASSERT( !m_children.contains( pathEntry->name() ) );
    m_children[pathEntry->name()] = std::move( pathEntry );
}

void RestPathEntry::addAction( std::unique_ptr<RestAction> action )
{
    auto verb = action->verb();
    CAFFA_ASSERT( !m_actions.contains( verb ) );
    m_actions[verb] = std::move( action );
}

std::pair<const RestPathEntry*, std::list<std::string>> RestPathEntry::findPathEntry( std::list<std::string> path ) const
{
    CAFFA_ASSERT( !path.empty() );
    auto currentLevel = path.front();

    if ( currentLevel == name() )
    {
        CAFFA_TRACE( "Found regular path entry " << currentLevel << " == " << name() << ", Rest of path is: "
                                                 << caffa::StringTools::join( path.begin(), path.end(), "/" ) );

        path.pop_front();
        if ( path.empty() )
        {
            return std::make_pair( this, path );
        }

        // Path is still not empty, need to look for children
        for ( const auto& [name, child] : m_children )
        {
            auto requestAndRemainingParameters = child->findPathEntry( path );
            if ( requestAndRemainingParameters.first ) return requestAndRemainingParameters;
        }
        return std::make_pair( nullptr, path );
    }
    else if ( matchesPathArgument( currentLevel ) )
    {
        CAFFA_TRACE( "Found path argument " << currentLevel << " == " << name() << ", Rest of path is: "
                                            << caffa::StringTools::join( path.begin(), path.end(), "/" ) );

        path.pop_front();
        std::list<std::string> parameters = { currentLevel };

        // Matches an argument in the path (such as an UUID)
        // Look for (optional) children
        for ( const auto& [name, child] : m_children )
        {
            auto [request, subParameters] = child->findPathEntry( path );
            if ( request )
            {
                for ( const auto& param : subParameters )
                {
                    parameters.push_back( param );
                }
                return std::make_pair( request, parameters );
            }
        }
        return std::make_pair( this, parameters );
    }

    return std::make_pair( nullptr, path );
}

std::list<const RestPathEntry*> RestPathEntry::children() const
{
    std::list<const RestPathEntry*> children;
    for ( const auto& [name, child] : m_children )
    {
        children.push_back( child.get() );
    }
    return children;
}

std::list<const RestAction*> RestPathEntry::actions() const
{
    std::list<const RestAction*> actions;
    for ( const auto& [verb, action] : m_actions )
    {
        actions.push_back( action.get() );
    }
    return actions;
}

nlohmann::json RestPathEntry::schema() const
{
    auto request = nlohmann::json::object();

    for ( const auto& [verb, action] : m_actions )
    {
        std::string verbString( http::to_string( verb ) );
        request[caffa::StringTools::tolower( verbString )] = action->schema();
    }

    return request;
}

bool RestPathEntry::requiresSession( http::verb verb ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        return false;
    }
    return it->second->requiresSession();
}

bool RestPathEntry::requiresAuthentication( http::verb verb ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        return false;
    }
    return it->second->requiresAuthentication();
}

void RestPathEntry::setPathArgumentMatcher( const std::function<bool( std::string )>& argumentMatcher )
{
    m_pathArgumentMatcher = argumentMatcher;
}

bool RestPathEntry::matchesPathArgument( const std::string& pathArgument ) const
{
    if ( m_pathArgumentMatcher )
    {
        return m_pathArgumentMatcher( pathArgument );
    }
    return false;
}

RequestFinder::RequestFinder( const RestPathEntry* rootPath )
    : m_rootPath( rootPath )
{
}

void RequestFinder::search()
{
    searchPath( m_rootPath, "/" + m_rootPath->name() );
}

const std::list<std::pair<std::string, const RestPathEntry*>>& RequestFinder::allPathEntriesWithActions()
{
    return m_allPathEntriesWithActions;
}

void RequestFinder::searchPath( const RestPathEntry* pathEntry, std::string currentPath )
{
    auto [request, pathArguments] = pathEntry->findPathEntry( { pathEntry->name() } );

    if ( request && !request->actions().empty() )
    {
        m_allPathEntriesWithActions.push_back( std::make_pair( currentPath, request ) );
    }

    for ( auto child : pathEntry->children() )
    {
        auto nextLevelPath = currentPath + "/" + child->name();
        searchPath( child, nextLevelPath );
    }
}
