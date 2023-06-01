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

    template <std::size_t... Is>
    Result execute( const nlohmann::json& args, std::index_sequence<Is...> ) const
    {
        return this->operator()( args[Is].get<ArgTypes>()... );
    }

    std::string execute( const std::string& jsonArguments ) const override
    {
        nlohmann::json json   = nlohmann::json::parse( jsonArguments );
        Result         result = execute( json, std::index_sequence_for<ArgTypes...>() );

        nlohmann::json jsonResult = nlohmann::json::object();
        jsonResult["type"]        = PortableDataType<Result>::name();
        jsonResult["value"]       = result;
        return jsonResult.dump();
    }

    Callback callback;
};

} // namespace caffa