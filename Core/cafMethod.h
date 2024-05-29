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
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafMethodHandle.h"
#include "cafObjectFactory.h"
#include "cafObjectJsonConversion.h"
#include "cafPortableDataType.h"
#include "cafSession.h"

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
    using Callback            = std::function<Result( ArgTypes... )>;
    using CallbackWithSession = std::function<Result( std::shared_ptr<Session>, ArgTypes... )>;

    Result operator()( std::shared_ptr<Session> session, ArgTypes... args ) const
    {
        if ( !m_callbackWithSession )
        {
            return this->operator()( args... ); // pass on to operator without session
        }

        CAFFA_ASSERT( m_callbackWithSession );
        CAFFA_ASSERT( !this->accessor() );
        return m_callbackWithSession( session, args... );
    }

    Result operator()( ArgTypes... args ) const
    {
        if ( auto accessor = this->accessor(); accessor )
        {
            auto serializedMethod = toJson( args... ).dump();
            CAFFA_DEBUG( "Serialized method: " << serializedMethod );
            auto serialisedResult = accessor->execute( serializedMethod );
            CAFFA_DEBUG( "Got serialized result: " << serialisedResult );
            return resultFromJsonString( serialisedResult, accessor->objectFactory() );
        }
        CAFFA_ASSERT( m_callback );
        return m_callback( args... );
    }

    /**
     * Execute the method from a JSON string and return result as a JSON string
     * @param jsonArgumentsString
     * @return a JSON result string
     */
    std::string execute( std::shared_ptr<Session> session, const std::string& jsonArgumentsString ) const override
    {
        return executeJson( session, nlohmann::json::parse( jsonArgumentsString ) ).dump();
    }

    std::string schema() const override { return this->jsonSchema().dump(); }

    Result resultFromJsonString( const std::string& jsonResultString, ObjectFactory* objectFactory ) const
    {
        CAFFA_DEBUG( "Attempting to get a value of type " << PortableDataType<Result>::jsonType()
                                                          << " from JSON: " << jsonResultString );
        nlohmann::json jsonResult;
        if ( !jsonResultString.empty() )
        {
            jsonResult = nlohmann::json::parse( jsonResultString );
        }
        return jsonToValue<Result>( jsonResult, objectFactory );
    }

    nlohmann::json toJson( ArgTypes... args ) const
    {
        auto jsonMethod = nlohmann::json::object();
        CAFFA_ASSERT( !keyword().empty() );

        constexpr std::size_t n = sizeof...( args );
        if constexpr ( n > 0 )
        {
            auto   jsonArguments = nlohmann::json::array();
            size_t i             = 0;

            // Fold expression
            // https://en.cppreference.com/w/cpp/language/fold
            (
                [&i, &args, &jsonArguments]
                {
                    jsonArguments.push_back( args );
                    i++;
                }(),
                ... );
            jsonMethod["positionalArguments"] = jsonArguments;
        }
        return jsonMethod;
    }

    nlohmann::json jsonSchema() const override
    {
        auto jsonMethod = nlohmann::json::object();
        CAFFA_ASSERT( !keyword().empty() );
        jsonMethod["type"] = "object";
        if ( !this->documentation().empty() )
        {
            jsonMethod["description"] = this->documentation();
        }
        auto jsonProperties    = nlohmann::json::object();
        auto jsonArgumentItems = jsonArgumentSchemaArray( std::index_sequence_for<ArgTypes...>() );

        if ( !jsonArgumentItems.empty() )
        {
            auto jsonpositionalArguments        = nlohmann::json::object();
            jsonpositionalArguments["type"]     = "array";
            jsonpositionalArguments["minItems"] = jsonArgumentItems.size();
            jsonpositionalArguments["maxItems"] = jsonArgumentItems.size();

            auto jsonNumberedArgumentItems      = nlohmann::json::array();
            auto jsonLabelledArguments          = nlohmann::json::object();
            jsonLabelledArguments["type"]       = "object";
            auto jsonLabelledArgumentProperties = nlohmann::json::object();
            for ( const nlohmann::json& argument : jsonArgumentItems )
            {
                CAFFA_ASSERT( argument.is_object() );
                auto keyword                            = argument["keyword"].get<std::string>();
                auto type                               = argument["type"];
                jsonLabelledArgumentProperties[keyword] = type;
                jsonNumberedArgumentItems.push_back( type );
            }
            jsonpositionalArguments["items"]      = jsonNumberedArgumentItems;
            jsonProperties["positionalArguments"] = jsonpositionalArguments;
            jsonLabelledArguments["properties"]   = jsonLabelledArgumentProperties;
            jsonProperties["labelledArguments"]   = jsonLabelledArguments;
        }
        if ( !PortableDataType<Result>::jsonType().empty() )
        {
            jsonProperties["returns"] = PortableDataType<Result>::jsonType();
        }

        jsonMethod["properties"] = jsonProperties;

        return jsonMethod;
    }

    void setCallback( Callback callback ) { this->m_callback = callback; }
    void setCallbackWithSession( CallbackWithSession callback ) { this->m_callbackWithSession = callback; }

private:
    template <typename ArgType>
        requires std::same_as<ArgType, void>
    static ArgType jsonToValue( const nlohmann::json& jsonData, ObjectFactory* objectFactory )
    {
        return;
    }

    template <typename ArgType>
        requires IsSharedPtr<ArgType>
    static ArgType jsonToValue( const nlohmann::json& jsonData, ObjectFactory* objectFactory )
    {
        JsonSerializer serializer( objectFactory );
        return std::dynamic_pointer_cast<typename ArgType::element_type>(
            serializer.createObjectFromString( jsonData.dump() ) );
    }

    template <typename ArgType>
        requires( not IsSharedPtr<ArgType> && not std::same_as<ArgType, void> )
    static ArgType jsonToValue( const nlohmann::json& jsonData, ObjectFactory* objectFactory )
    {
        return jsonData.get<ArgType>();
    }

    template <typename ReturnType, std::size_t... Is>
        requires std::same_as<ReturnType, void>
    nlohmann::json executeJson( std::shared_ptr<Session> session, const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        this->operator()( session, jsonToValue<ArgTypes>( args[Is], nullptr )... );

        nlohmann::json returnValue = nlohmann::json::object();
        return returnValue;
    }

    template <typename ReturnType, std::size_t... Is>
        requires( not std::same_as<ReturnType, void> )
    nlohmann::json executeJson( std::shared_ptr<Session> session, const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        return this->operator()( session, jsonToValue<ArgTypes>( args[Is], nullptr )... );
    }

    nlohmann::json executeJson( std::shared_ptr<Session> session, const nlohmann::json& jsonMethod ) const
    {
        auto jsonArguments = nlohmann::json::array();
        if ( jsonMethod.contains( "positionalArguments" ) )
        {
            jsonArguments = jsonMethod["positionalArguments"];
        }
        else if ( jsonMethod.contains( "labelledArguments" ) )
        {
            jsonArguments = jsonMethod["labelledArguments"];
            sortArguments( jsonArguments, argumentNames() );
        }
        auto expectedSize = jsonArgumentSchemaArray( std::index_sequence_for<ArgTypes...>() ).size();
        if ( jsonArguments.size() != expectedSize )
        {
            throw std::runtime_error( "Wrong number of arguments! Got " + std::to_string( jsonArguments.size() ) +
                                      ", Expected " + std::to_string( expectedSize ) );
        }
        return this->executeJson<Result>( session, jsonArguments, std::index_sequence_for<ArgTypes...>() );
    }

    void sortArguments( nlohmann::json& jsonMap, const std::vector<std::string>& argumentNames ) const
    {
        nlohmann::json sortedArray = nlohmann::json::array();
        for ( const auto& argumentName : argumentNames )
        {
            auto it = jsonMap.find( argumentName );
            if ( it != jsonMap.end() )
            {
                sortedArray.push_back( *it );
                jsonMap.erase( it );
            }
        }
        // All unnamed arguments
        sortedArray.insert( sortedArray.end(), jsonMap.begin(), jsonMap.end() );
        jsonMap.swap( sortedArray );
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
    nlohmann::json jsonArgumentSchemaArray( std::index_sequence<Is...> ) const
    {
        auto jsonArguments = nlohmann::json::array();
        argumentHelper( jsonArguments, PortableDataType<ArgTypes>::jsonType()... );
        return jsonArguments;
    }

private:
    Callback            m_callback;
    CallbackWithSession m_callbackWithSession;
};

} // namespace caffa