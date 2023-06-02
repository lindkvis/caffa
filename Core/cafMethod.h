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

#include <nlohmann/json.hpp>

#include <functional>

namespace caffa
{
class ObjectHandle;

template <class CallbackT>
class Method : public MethodHandle
{
public:
    std::function<CallbackT> callback;
};

template <class Result, typename... ArgTypes>
class Method<Result( ArgTypes... )> : public MethodHandle
{
public:
    using Callback = std::function<Result( ArgTypes... )>;

    Result operator()( ArgTypes... args ) const
    {
        CAFFA_ASSERT( callback );
        return callback( args... );
    }

    std::string execute( const std::string& jsonArgumentsString ) const override
    {
        return executeJson( nlohmann::json::parse( jsonArgumentsString ) ).dump();
    }

    template <typename T>
    static std::string dataType( const T& value )
    {
        return PortableDataType<T>::name();
    }

    static nlohmann::json jsonArguments( ArgTypes... args )
    {
        auto jsonArray = nlohmann::json::array();
        (
            [&]
            {
                nlohmann::json jsonArg = nlohmann::json::object();
                jsonArg["type"]        = dataType( args );
                jsonArg["value"]       = args;
                jsonArray.push_back( jsonArg );
            }(),
            ... );
        return jsonArray;
    }

    static nlohmann::json jsonReturnType()
    {
        nlohmann::json jsonResult = nlohmann::json::object();
        jsonResult["type"]        = PortableDataType<Result>::name();
        return jsonResult;
    }

    void setCallback( Callback callback ) { this->callback = callback; }

private:
    template <std::size_t... Is>
    Result executeJson( const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        return this->operator()( args[Is].get<ArgTypes>()... );
    }

    nlohmann::json executeJson( const nlohmann::json& jsonArguments ) const
    {
        Result result = this->executeJson( jsonArguments, std::index_sequence_for<ArgTypes...>() );

        nlohmann::json jsonResult = jsonReturnType();
        jsonResult["value"]       = result;
        return jsonResult;
    }

private:
    Callback callback;
};

} // namespace caffa