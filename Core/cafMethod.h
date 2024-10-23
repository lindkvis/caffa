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
#include "cafJsonDataType.h"
#include "cafJsonDataTypeConversion.h"
#include "cafJsonDefinitions.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafMethodHandle.h"
#include "cafObjectFactory.h"
#include "cafSession.h"

#include <functional>

namespace caffa
{
class ObjectHandle;

template <class CallbackT>
class Method : public MethodHandle
{
protected:
    std::function<CallbackT> m_callback;
};

template <typename Result, typename... ArgTypes>
class Method<Result( ArgTypes... )> final : public MethodHandle
{
public:
    using Callback            = std::function<Result( ArgTypes... )>;
    using CallbackWithSession = std::function<Result( std::shared_ptr<Session>, ArgTypes... )>;

    Method()                               = default;
    Method( const Method& rhs )            = delete;
    Method& operator=( const Method& rhs ) = delete;

    [[nodiscard]] Result operator()( std::shared_ptr<Session> session, ArgTypes... args ) const
    {
        CAFFA_ASSERT( ( m_callback || m_callbackWithSession ) && "Method has no callback!" );

        if ( !m_callbackWithSession )
        {
            return this->operator()( args... ); // pass on to operator without session
        }

        CAFFA_ASSERT( m_callbackWithSession );
        CAFFA_ASSERT( !this->accessor() );
        return m_callbackWithSession( session, args... );
    }

    [[nodiscard]] Result operator()( ArgTypes... args ) const
    {
        CAFFA_ASSERT( ( m_callback || m_callbackWithSession ) && "Method has no callback!" );

        if ( auto accessor = this->accessor(); accessor )
        {
            auto serializedMethod = json::dump( toJson( args... ) );
            CAFFA_TRACE( "Serialized method: " << serializedMethod );
            auto serialisedResult = accessor->execute( serializedMethod );
            CAFFA_TRACE( "Got serialized result: " << serialisedResult );
            return resultFromJsonString( serialisedResult, accessor->objectFactory() );
        }

        if ( !m_callback )
        {
            throw std::runtime_error( "Method needs to be called with a session as the first argument!" );
        }
        return m_callback( args... );
    }

    /**
     * Execute the method from a JSON string and return result as a JSON string
     * @param session
     * @param jsonArgumentsString
     * @return a JSON result string
     */
    [[nodiscard]] std::string execute( std::shared_ptr<Session> session, const std::string& jsonArgumentsString ) const override
    {
        return json::dump( executeJson( session, json::parse( jsonArgumentsString ) ) );
    }

    [[nodiscard]] std::string schema() const override { return json::dump( this->jsonSchema() ); }

    [[nodiscard]] Result resultFromJsonString( const std::string& jsonResultString, ObjectFactory* objectFactory ) const
    {
        json::value jsonResult;
        if ( !jsonResultString.empty() )
        {
            jsonResult = json::parse( jsonResultString );
        }
        return jsonToValue<Result>( jsonResult, objectFactory );
    }

    [[nodiscard]] json::value toJson( ArgTypes... args ) const
    {
        auto jsonMethod = json::object();
        CAFFA_ASSERT( !keyword().empty() );

        if constexpr ( sizeof...( args ) > 0 )
        {
            auto   jsonArguments = json::array();
            size_t i             = 0;

            // Fold expression
            // https://en.cppreference.com/w/cpp/language/fold
            //            ( jsonArguments.push_back( std::forward<ArgTypes>( args ) ), ... );
            (
                [&i, &args, &jsonArguments]
                {
                    jsonArguments.push_back( json::to_json( args ) );
                    i++;
                }(),
                ... );
            jsonMethod["positionalArguments"] = jsonArguments;
        }
        return jsonMethod;
    }

    [[nodiscard]] json::object jsonSchema() const
    {
        auto jsonMethod = json::object();
        CAFFA_ASSERT( !keyword().empty() );
        jsonMethod["type"] = "object";
        if ( !this->documentation().empty() )
        {
            jsonMethod["description"] = this->documentation();
        }
        auto jsonProperties    = json::object();
        auto jsonArgumentItems = jsonArgumentSchemaArray( std::index_sequence_for<ArgTypes...>() );

        if ( !jsonArgumentItems.empty() )
        {
            auto jsonPositionalArguments        = json::object();
            jsonPositionalArguments["type"]     = "array";
            jsonPositionalArguments["minItems"] = jsonArgumentItems.size();
            jsonPositionalArguments["maxItems"] = jsonArgumentItems.size();

            auto jsonNumberedArgumentItems   = json::array();
            auto jsonNamedArguments          = json::object();
            jsonNamedArguments["type"]       = "object";
            auto jsonNamedArgumentProperties = json::object();
            for ( const json::value& argument : jsonArgumentItems )
            {
                CAFFA_ASSERT( argument.is_object() );
                auto        keyword                  = json::from_json<std::string>( argument.at( "keyword" ) );
                const auto& type                     = argument.at( "type" );
                jsonNamedArgumentProperties[keyword] = type;
                jsonNumberedArgumentItems.push_back( type );
            }
            jsonPositionalArguments["items"]      = jsonNumberedArgumentItems;
            jsonProperties["positionalArguments"] = jsonPositionalArguments;
            jsonNamedArguments["properties"]      = jsonNamedArgumentProperties;
            jsonProperties["labelledArguments"]   = jsonNamedArguments;
        }
        if ( !JsonDataType<Result>::jsonType().empty() )
        {
            jsonProperties["returns"] = JsonDataType<Result>::jsonType();
        }

        jsonMethod["properties"] = jsonProperties;

        return jsonMethod;
    }

    void setCallback( Callback callback ) { this->m_callback = callback; }
    void setCallbackWithSession( CallbackWithSession callback ) { this->m_callbackWithSession = callback; }

private:
    template <typename ArgType>
        requires std::same_as<ArgType, void>
    static ArgType jsonToValue( const json::value& jsonData, ObjectFactory* objectFactory )
    {
        return void();
    }

    template <typename ArgType>
        requires IsSharedPtr<ArgType>
    [[nodiscard]] static ArgType jsonToValue( const json::value& jsonData, ObjectFactory* objectFactory )
    {
        const JsonSerializer serializer( objectFactory );
        return std::dynamic_pointer_cast<typename ArgType::element_type>(
            serializer.createObjectFromJson( jsonData.as_object() ) );
    }

    template <typename ArgType>
        requires( not IsSharedPtr<ArgType> && not std::same_as<ArgType, void> )
    [[nodiscard]] static ArgType jsonToValue( const json::value& jsonData, ObjectFactory* objectFactory )
    {
        return json::from_json<ArgType>( jsonData );
    }

    template <typename ReturnType, std::size_t... Is>
        requires std::same_as<ReturnType, void>
    [[nodiscard]] json::value
        executeJson( std::shared_ptr<Session> session, const json::array& args, std::index_sequence<Is...> ) const
    {
        this->operator()( session, jsonToValue<ArgTypes>( args[Is], nullptr )... );

        json::value returnValue = json::object();
        return returnValue;
    }

    template <typename ReturnType, std::size_t... Is>
        requires( not std::same_as<ReturnType, void> )
    [[nodiscard]] json::value
        executeJson( std::shared_ptr<Session> session, const json::array& args, std::index_sequence<Is...> ) const
    {
        auto res = this->operator()( session, jsonToValue<ArgTypes>( args[Is], nullptr )... );
        return json::to_json( res );
    }

    [[nodiscard]] json::value executeJson( std::shared_ptr<Session> session, const json::value& jsonValue ) const
    {
        const auto* jsonMethod = jsonValue.if_object();

        auto jsonArguments = json::array();
        if ( jsonMethod )
        {
            if ( const auto it = jsonMethod->find( "positionalArguments" );
                 it != jsonMethod->end() && it->value().is_array() )
            {
                jsonArguments = it->value().get_array();
            }
            if ( const auto it = jsonMethod->find( "labelledArguments" );
                 it != jsonMethod->end() && it->value().is_object() )
            {
                auto jsonNamedArguments = it->value().get_object();
                jsonArguments           = sortArguments( jsonNamedArguments, argumentNames() );
            }
        }

        auto expectedSize = jsonArgumentSchemaArray( std::index_sequence_for<ArgTypes...>() ).size();
        if ( jsonArguments.size() != expectedSize )
        {
            throw std::runtime_error( "Wrong number of arguments! Got " + std::to_string( jsonArguments.size() ) +
                                      ", Expected " + std::to_string( expectedSize ) );
        }
        return this->template executeJson<Result>( session, jsonArguments, std::index_sequence_for<ArgTypes...>() );
    }

    static json::array sortArguments( json::object& jsonMap, const std::vector<std::string>& argumentNames )
    {
        json::array sortedArray;
        for ( const auto& argumentName : argumentNames )
        {
            if ( const auto it = jsonMap.find( argumentName ); it != jsonMap.end() )
            {
                sortedArray.push_back( it->value() );
                jsonMap.erase( it );
            }
        }
        std::vector<std::string> unknownArguments;
        // All unknown arguments
        for ( const auto& unknownArgument : jsonMap )
        {
            unknownArguments.push_back( ( unknownArgument.key() ) );
        }
        if ( !unknownArguments.empty() )
        {
            throw std::runtime_error( "Unknown arguments: " +
                                      StringTools::join( unknownArguments.begin(), unknownArguments.end(), ", " ) );
        }

        return sortedArray;
    }

    template <typename... T>
    void argumentHelper( json::array& jsonArguments, const T&... argumentTypes ) const
    {
        const auto&           argumentNames = this->argumentNames();
        constexpr std::size_t n             = sizeof...( argumentTypes );
        if constexpr ( n > 0 )
        {
            size_t i = 0;
            (
                [&]
                {
                    json::object jsonArg;
                    if ( i < argumentNames.size() )
                    {
                        jsonArg["keyword"] = argumentNames[i];
                    }
                    jsonArg["type"] = argumentTypes;
                    jsonArguments.push_back( jsonArg );
                    i++;
                }(),
                ... );
        }
    }

    template <std::size_t... Is>
    json::array jsonArgumentSchemaArray( std::index_sequence<Is...> ) const
    {
        json::array jsonArguments;
        argumentHelper( jsonArguments, JsonDataType<ArgTypes>::jsonType()... );
        return jsonArguments;
    }

    Callback            m_callback;
    CallbackWithSession m_callbackWithSession;
};

} // namespace caffa
