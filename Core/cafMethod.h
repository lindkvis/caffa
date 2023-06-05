// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) Kontur As
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
// ##################################################################################################
#pragma once

#include "cafAssert.h"
#include "cafLogger.h"
#include "cafMethodHandle.h"
#include "cafPortableDataType.h"

#include "cafObjectJsonSpecializations.h"

#include <nlohmann/json.hpp>

#include <functional>

namespace caffa
{
class ObjectHandle;

template <class CallbackT>
class Method : public MethodHandle
{
public:
    std::function<CallbackT> m_callback;
};

template <typename Result, typename... ArgTypes>
class Method<Result( ArgTypes... )> : public MethodHandle
{
public:
    using Callback = std::function<Result( ArgTypes... )>;

    Result operator()( ArgTypes... args ) const
    {
        CAFFA_ASSERT( m_callback );
        return m_callback( args... );
    }

    std::string execute( const std::string& jsonArgumentsString ) const override
    {
        return executeJson( nlohmann::json::parse( jsonArgumentsString ) ).dump();
    }

    nlohmann::json toJson( ArgTypes... args ) const
    {
        auto jsonMethod = nlohmann::json::object();
        CAFFA_ASSERT( !name().empty() );

        jsonMethod["keyword"]     = name();
        auto        jsonArguments = nlohmann::json::array();
        const auto& argumentNames = this->argumentNames();
        size_t      i             = 0;

        // Fold expression
        // https://en.cppreference.com/w/cpp/language/fold
        (
            [&]
            {
                nlohmann::json jsonArg = nlohmann::json::object();
                jsonArg["keyword"]     = i < argumentNames.size() ? argumentNames[i]
                                                                  : std::string( "arg" ) + std::to_string( i );
                jsonArg["type"]        = PortableDataType<ArgTypes>::name();
                jsonArg["value"]       = args;
                jsonArguments.push_back( jsonArg );
                i++;
            }(),
            ... );
        jsonMethod["arguments"] = jsonArguments;
        jsonMethod["returns"]   = PortableDataType<Result>::name();

        return jsonMethod;
    }

    nlohmann::json jsonSkeleton() const
    {
        auto jsonMethod = nlohmann::json::object();
        CAFFA_ASSERT( !name().empty() );

        jsonMethod["keyword"] = name();
        auto jsonArguments    = jsonArgumentArray( std::index_sequence_for<ArgTypes...>() );
        if ( !jsonArguments.empty() )
        {
            jsonMethod["arguments"] = jsonArguments;
        }
        jsonMethod["returns"] = PortableDataType<Result>::name();

        return jsonMethod;
    }

    void setCallback( Callback callback ) { this->m_callback = callback; }

private:
    template <typename ArgType>
        requires IsSharedPtr<ArgType>
    static ArgType jsonToArgument( const nlohmann::json& value )
    {
        JsonSerializer serializer;
        return std::dynamic_pointer_cast<typename ArgType::element_type>(
            serializer.createObjectFromString( value.dump() ) );
    }

    template <typename ArgType>
        requires( not IsSharedPtr<ArgType> )
    static ArgType jsonToArgument( const nlohmann::json& value )
    {
        return value.get<ArgType>();
    }

    template <typename ReturnType, std::size_t... Is>
        requires std::same_as<ReturnType, void>
    nlohmann::json executeJson( const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        this->operator()( jsonToArgument<ArgTypes>( args[Is]["value"] )... );

        nlohmann::json returnValue = nlohmann::json::object();
        returnValue["type"]        = PortableDataType<Result>::name();
        return returnValue;
    }

    template <typename ReturnType, std::size_t... Is>
        requires( not std::same_as<ReturnType, void> )
    nlohmann::json executeJson( const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        nlohmann::json returnValue = nlohmann::json::object();
        returnValue["type"]        = PortableDataType<Result>::name();
        returnValue["value"]       = this->operator()( jsonToArgument<ArgTypes>( args[Is]["value"] )... );
        return returnValue;
    }

    nlohmann::json executeJson( const nlohmann::json& jsonMethod ) const
    {
        nlohmann::json jsonArguments = jsonMethod["arguments"];
        return this->executeJson<Result>( jsonArguments, std::index_sequence_for<ArgTypes...>() );
    }

    template <typename... T>
    void argumentHelper( nlohmann::json& jsonArguments, const T&... argumentTypes ) const
    {
        const auto& argumentNames = this->argumentNames();
        size_t      i             = 0;
        (
            [&]
            {
                nlohmann::json jsonArg = nlohmann::json::object();
                jsonArg["name"]        = argumentNames[i];
                jsonArg["type"]        = argumentTypes;
                jsonArguments.push_back( jsonArg );
                i++;
            }(),
            ... );
    }

    template <std::size_t... Is>
    nlohmann::json jsonArgumentArray( std::index_sequence<Is...> ) const
    {
        auto jsonArguments = nlohmann::json::array();
        argumentHelper( jsonArguments, PortableDataType<ArgTypes>::name()... );
        return jsonArguments;
    }

private:
    Callback m_callback;
};

} // namespace caffa