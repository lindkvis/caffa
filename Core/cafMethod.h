// ##################################################################################################
//
//    CAFFA
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
        if ( auto accessor = this->accessor(); accessor )
        {
            auto serializedMethod = toJson( args... ).dump();
            CAFFA_DEBUG( "Serialized method: " << serializedMethod );
            auto serialisedResult = accessor->execute( serializedMethod );
            CAFFA_DEBUG( "Got serialized result: " << serialisedResult );
            return resultFromJsonString( serialisedResult );
        }
        return m_callback( args... );
    }

    /**
     * Execute the method from a JSON string and return result as a JSON string
     * @param jsonArgumentsString
     * @return a JSON result string
     */
    std::string execute( const std::string& jsonArgumentsString ) const override
    {
        return executeJson( nlohmann::json::parse( jsonArgumentsString ) ).dump();
    }

    std::string schema() const override { return this->jsonSkeleton().dump(); }

    Result resultFromJsonString( const std::string& jsonResultString ) const
    {
        CAFFA_DEBUG( "Attempting to get a value of type " << PortableDataType<Result>::name()
                                                          << " from JSON: " << jsonResultString );
        auto jsonResult = nlohmann::json::parse( jsonResultString );
        return jsonToValue<Result>( jsonResult );
    }

    nlohmann::json toJson( ArgTypes... args ) const
    {
        auto jsonMethod = nlohmann::json::object();
        CAFFA_ASSERT( !name().empty() );

        jsonMethod["keyword"] = name();

        constexpr std::size_t n = sizeof...( args );
        if constexpr ( n > 0 )
        {
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
        }

        jsonMethod["returns"] = PortableDataType<Result>::name();

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
        requires std::same_as<ArgType, void>
    static ArgType jsonToValue( const nlohmann::json& value )
    {
        return;
    }

    template <typename ArgType>
        requires IsSharedPtr<ArgType>
    static ArgType jsonToValue( const nlohmann::json& value )
    {
        JsonSerializer serializer;
        return std::dynamic_pointer_cast<typename ArgType::element_type>(
            serializer.createObjectFromString( value["value"].dump() ) );
    }

    template <typename ArgType>
        requires( not IsSharedPtr<ArgType> && not std::same_as<ArgType, void> )
    static ArgType jsonToValue( const nlohmann::json& value )
    {
        return value["value"].get<ArgType>();
    }

    template <typename ReturnType, std::size_t... Is>
        requires std::same_as<ReturnType, void>
    nlohmann::json executeJson( const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        this->operator()( jsonToValue<ArgTypes>( args[Is] )... );

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
        returnValue["value"]       = this->operator()( jsonToValue<ArgTypes>( args[Is] )... );
        return returnValue;
    }

    nlohmann::json executeJson( const nlohmann::json& jsonMethod ) const
    {
        auto jsonArguments = nlohmann::json::array();
        if ( jsonMethod.contains( "arguments" ) )
        {
            jsonArguments = jsonMethod["arguments"];
            sortArguments( jsonArguments, argumentNames() );
        }
        return this->executeJson<Result>( jsonArguments, std::index_sequence_for<ArgTypes...>() );
    }

    void sortArguments( nlohmann::json& jsonArray, const std::vector<std::string>& argumentNames ) const
    {
        nlohmann::json sortedArray = nlohmann::json::array();
        for ( const auto& argumentName : argumentNames )
        {
            auto it = std::find_if( jsonArray.begin(),
                                    jsonArray.end(),
                                    [&argumentName]( auto jsonElement )
                                    { return jsonElement["keyword"] == argumentName; } );
            if ( it != jsonArray.end() )
            {
                sortedArray.push_back( *it );
                jsonArray.erase( it );
            }
        }
        // All unnamed arguments
        sortedArray.insert( sortedArray.end(), jsonArray.begin(), jsonArray.end() );
        jsonArray.swap( sortedArray );
    }

    template <typename... T>
    void argumentHelper( nlohmann::json& jsonArguments, const T&... argumentTypes ) const
    {
        const auto&           argumentNames = this->argumentNames();
        constexpr std::size_t n             = sizeof...( argumentTypes );
        if constexpr ( n > 0 )
        {
            size_t i = 0;
            (
                [&]
                {
                    nlohmann::json jsonArg = nlohmann::json::object();
                    jsonArg["keyword"]     = argumentNames[i];
                    jsonArg["type"]        = argumentTypes;
                    jsonArguments.push_back( jsonArg );
                    i++;
                }(),
                ... );
        }
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