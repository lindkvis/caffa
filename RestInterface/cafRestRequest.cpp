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

using namespace caffa::rpc;

RestResponse::RestResponse( const std::string& description, const std::string& contentType, const std::string& schemaPath )
    : m_description( description )
    , m_contentType( contentType )
    , m_schemaPath( schemaPath )
{
}

nlohmann::json RestResponse::schema() const
{
    auto response = nlohmann::json{ { "description", m_description } };

    if ( !m_contentType.empty() && !m_schemaPath.empty() )
    {
        auto content           = nlohmann::json::object();
        content[m_contentType] = { { "schema", { { "$ref", m_schemaPath } } } };
        response["content"]    = content;
    }

    return response;
}

std::unique_ptr<RestResponse> RestResponse::plainError()
{
    return std::make_unique<RestResponse>( "A Service Error Message", "text/plain", "#/components/error_schemas/PlainError" );
}

RestAction::RestAction( http::verb verb, const std::string& summary, const std::string& operationId, const Callback& callback )
    : m_verb( verb )
    , m_summary( summary )
    , m_operationId( operationId )
    , m_callback( callback )
    , m_requiresSession( true )
    , m_requiresAuthentication( true )
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
        auto statusString       = std::to_string( static_cast<unsigned>( status ) );
        responses[statusString] = response->schema();
    }

    auto action = nlohmann::json{ { "summary", m_summary },
                                  { "operationId", m_operationId },
                                  { "responses", responses },
                                  { "tags", tagList } };
    return action;
}

RestAction::ServiceResponse RestAction::perform( const nlohmann::json& queryParams, const nlohmann::json& body ) const
{
    return m_callback( queryParams, body );
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

RestPathEntry::RestPathEntry( const std::string& name )
    : m_name( name )
{
}

const std::string& RestPathEntry::name() const
{
    return m_name;
}

RestPathEntry::ServiceResponse
    RestPathEntry::perform( http::verb verb, const nlohmann::json& queryParams, const nlohmann::json& body ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        throw std::runtime_error( "Could not find the verb " + std::string( http::to_string( verb ) ) + " in request " +
                                  name() );
    }

    return it->second->perform( queryParams, body );
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

const RestPathEntry* RestPathEntry::findPathEntry( std::list<std::string> path ) const
{
    CAFFA_ASSERT( !path.empty() );
    auto currentLevel = path.front();
    path.pop_front();

    if ( currentLevel == name() )
    {
        if ( !path.empty() )
        {
            for ( const auto& [name, child] : m_children )
            {
                auto request = child->findPathEntry( path );
                if ( request ) return request;
            }
        }
        else
        {
            return this;
        }
    }
    return nullptr;
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
        request[verbString] = action->schema();
    }

    return request;
}

bool RestPathEntry::requiresSession( http::verb verb ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        throw std::runtime_error( "Could not find the verb " + std::string( http::to_string( verb ) ) + " in request " +
                                  name() );
    }
    return it->second->requiresSession();
}

bool RestPathEntry::requiresAuthentication( http::verb verb ) const
{
    auto it = m_actions.find( verb );
    if ( it == m_actions.end() )
    {
        throw std::runtime_error( "Could not find the verb " + std::string( http::to_string( verb ) ) + " in request " +
                                  name() );
    }
    return it->second->requiresAuthentication();
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
    auto request = pathEntry->findPathEntry( { pathEntry->name() } );

    if ( request && !request->actions().empty() )
    {
        m_allPathEntriesWithActions.push_back( std::make_pair( currentPath, request ) );
    }

    for ( auto child : pathEntry->children() )
    {
        auto nextLevelPath = currentPath + "/" + child->name();
        CAFFA_INFO( "FOUND PATH ENTRY: " << nextLevelPath << " with " << child->actions().size() << " actions" );
        searchPath( child, nextLevelPath );
    }
}
