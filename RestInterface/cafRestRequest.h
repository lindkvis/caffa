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
#pragma once

#include "cafRestServiceInterface.h"

#include <nlohmann/json.hpp>

#include <boost/beast/http.hpp>

#include <functional>
#include <list>
#include <map>
#include <string>

namespace http = boost::beast::http;

namespace caffa::rpc
{

class RestResponse
{
public:
    RestResponse( const std::string& description, const std::string& contentType = "", const std::string& schemaPath = "" );
    nlohmann::json schema() const;

    static std::unique_ptr<RestResponse> plainError();

private:
    std::string m_description;
    std::string m_contentType;
    std::string m_schemaPath;
};

class RestAction
{
public:
    using ServiceResponse = RestServiceInterface::ServiceResponse;
    using Callback = std::function<ServiceResponse( const nlohmann::json& queryParams, const nlohmann::json& body )>;

    RestAction( http::verb verb, const std::string& summary, const std::string& operationId, const Callback& callback );

    http::verb     verb() const;
    void           addTag( const std::string& tag );
    void           addResponse( http::status status, std::unique_ptr<RestResponse> response );
    nlohmann::json schema() const;

    ServiceResponse perform( const nlohmann::json& queryParams, const nlohmann::json& body ) const;

    void setRequiresSession( bool requiresSession );
    void setRequiresAuthentication( bool requiresAuthentication );

    bool requiresSession() const;
    bool requiresAuthentication() const;

private:
    http::verb  m_verb;
    std::string m_summary;
    std::string m_operationId;
    Callback    m_callback;
    bool        m_requiresSession;
    bool        m_requiresAuthentication;

    std::list<std::string>                                m_tags;
    std::map<http::status, std::unique_ptr<RestResponse>> m_responses;
};

class RestPathEntry
{
public:
    using ServiceResponse = RestServiceInterface::ServiceResponse;

public:
    RestPathEntry( const std::string& name );
    const std::string& name() const;
    ServiceResponse    perform( http::verb verb, const nlohmann::json& queryParams, const nlohmann::json& body ) const;

    void addEntry( std::unique_ptr<RestPathEntry> pathEntry );
    void addAction( std::unique_ptr<RestAction> action );

    nlohmann::json schema() const;

    const RestPathEntry* findPathEntry( std::list<std::string> path ) const;

    std::list<const RestAction*>    actions() const;
    std::list<const RestPathEntry*> children() const;

    bool requiresSession( http::verb verb ) const;
    bool requiresAuthentication( http::verb verb ) const;

private:
    std::string                                           m_name;
    std::map<std::string, std::unique_ptr<RestPathEntry>> m_children;
    std::map<http::verb, std::unique_ptr<RestAction>>     m_actions;
};

class RequestFinder
{
public:
    RequestFinder( const RestPathEntry* rootPath );

    void search();

    const std::list<std::pair<std::string, const RestPathEntry*>>& allPathEntriesWithActions();

private:
    void searchPath( const RestPathEntry* pathEntry, std::string currentPath );

private:
    const RestPathEntry*                                    m_rootPath;
    std::list<std::pair<std::string, const RestPathEntry*>> m_allPathEntriesWithActions;
};

} // namespace caffa::rpc