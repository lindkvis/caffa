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

#include "cafJsonConversionHelpers.h"
#include "cafJsonDataType.h"
#include "cafRestServiceInterface.h"

#include <nlohmann/json.hpp>

#include <boost/beast/http.hpp>

#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>

namespace http = boost::beast::http;

namespace caffa::rpc
{

class RestResponse
{
public:
    RestResponse( const nlohmann::json& contentSchema, const std::string& description );

    nlohmann::json schema() const;

    static std::unique_ptr<RestResponse> emptyResponse( const std::string& description );
    static std::unique_ptr<RestResponse> plainErrorResponse();
    static std::unique_ptr<RestResponse> objectResponse( const std::string& schemaPath, const std::string& description );
    static std::unique_ptr<RestResponse> objectArrayResponse( const std::string& schemaPath,
                                                              const std::string& description );

    std::unique_ptr<RestResponse> clone() const;

private:
    nlohmann::json m_content;
    std::string    m_description;
};

class RestParameter
{
public:
    enum class Location
    {
        PATH,
        QUERY
    };

    RestParameter( const std::string& name, Location location, bool required, const std::string& description );
    virtual ~RestParameter();

    virtual nlohmann::json                 schema() const = 0;
    virtual std::unique_ptr<RestParameter> clone() const  = 0;

protected:
    std::string m_name;
    Location    m_location;
    bool        m_required;
    std::string m_description;
};

template <typename DataType>
class RestTypedParameter : public RestParameter
{
public:
    RestTypedParameter( const std::string& name, Location location, bool required, const std::string& description )
        : RestParameter( name, location, required, description )
    {
    }

    void setDefaultValue( DataType defaultValue ) { m_defaultValue = defaultValue; }

    nlohmann::json schema() const override
    {
        std::string    locString = ( m_location == Location::PATH ) ? "path" : "query";
        nlohmann::json schema    = nlohmann::json{ { "name", m_name },
                                                   { "in", locString },
                                                   { "required", m_required },
                                                   { "description", m_description },
                                                   { "schema", caffa::JsonDataType<DataType>::jsonType() } };
        if ( m_defaultValue )
        {
            schema["default"] = *m_defaultValue;
        }
        return schema;
    }

    std::unique_ptr<RestParameter> clone() const override
    {
        auto clone = std::make_unique<RestTypedParameter<DataType>>( m_name, m_location, m_required, m_description );
        if ( m_defaultValue )
        {
            clone->setDefaultValue( *m_defaultValue );
        }
        return clone;
    }

private:
    std::optional<DataType> m_defaultValue;
};

class RestAction
{
public:
    using ServiceResponse = RestServiceInterface::ServiceResponse;
    using Callback =
        std::function<ServiceResponse( http::verb, const std::list<std::string>&, const nlohmann::json&, const nlohmann::json& )>;

    RestAction( http::verb verb, const std::string& summary, const std::string& operationId, const Callback& callback );

    http::verb verb() const;
    void       addTag( const std::string& tag );

    void           addParameter( std::unique_ptr<RestParameter> parameter );
    void           addResponse( http::status status, std::unique_ptr<RestResponse> response );
    nlohmann::json schema() const;

    ServiceResponse perform( const std::list<std::string>& pathArguments,
                             const nlohmann::json&         queryParams,
                             const nlohmann::json&         body ) const;

    void setRequiresSession( bool requiresSession );
    void setRequiresAuthentication( bool requiresAuthentication );

    bool requiresSession() const;
    bool requiresAuthentication() const;

    void setRequestBodySchema( const nlohmann::json& requestBodySchema );

private:
    http::verb  m_verb;
    std::string m_summary;
    std::string m_operationId;
    Callback    m_callback;
    bool        m_requiresSession;
    bool        m_requiresAuthentication;

    std::list<std::string>                                m_tags;
    std::list<std::unique_ptr<RestParameter>>             m_parameters;
    std::map<http::status, std::unique_ptr<RestResponse>> m_responses;

    nlohmann::json m_requestBodySchema;
};

class RestPathEntry
{
public:
    using ServiceResponse = RestServiceInterface::ServiceResponse;

public:
    RestPathEntry( const std::string& name );
    const std::string& name() const;
    ServiceResponse    perform( http::verb                    verb,
                                const std::list<std::string>& pathArguments,
                                const nlohmann::json&         queryParams,
                                const nlohmann::json&         body ) const;

    void addEntry( std::unique_ptr<RestPathEntry> pathEntry );
    void addAction( std::unique_ptr<RestAction> action );

    nlohmann::json schema() const;

    std::pair<const RestPathEntry*, std::list<std::string>> findPathEntry( std::list<std::string> path ) const;

    std::list<const RestAction*>    actions() const;
    std::list<const RestPathEntry*> children() const;

    bool requiresSession( http::verb verb ) const;
    bool requiresAuthentication( http::verb verb ) const;

    void setPathArgumentMatcher( const std::function<bool( std::string )>& argumentMatcher );

private:
    bool matchesPathArgument( const std::string& pathArgument ) const;

    std::string                                           m_name;
    std::map<std::string, std::unique_ptr<RestPathEntry>> m_children;
    std::map<http::verb, std::unique_ptr<RestAction>>     m_actions;

    std::function<bool( std::string )> m_pathArgumentMatcher;
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