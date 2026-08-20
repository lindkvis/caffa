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

#include "cafMethodHandle.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace caffa
{

template <class CallbackT>
class Method : public MethodHandle
{
protected:
    std::function<CallbackT> m_callback;
};

/**
 * A named, documented callable member of an Object.
 *
 * Methods are invoked directly in C++. Remote dispatch, JSON (de)serialisation of arguments
 * and JSON schema generation used to live here to support the CAFFA REST interface, and were
 * removed with it.
 */
template <typename Result, typename... ArgTypes>
class Method<Result( ArgTypes... )> final : public MethodHandle
{
public:
    using Callback = std::function<Result( ArgTypes... )>;

    Method()                               = default;
    Method( const Method& rhs )            = delete;
    Method& operator=( const Method& rhs ) = delete;

    Result operator()( ArgTypes... args ) const
    {
        if ( !m_callback )
        {
            throw std::runtime_error( "Method " + this->keyword() + "() has no callback" );
        }
        return m_callback( args... );
    }

    void setCallback( Callback callback ) { this->m_callback = std::move( callback ); }

private:
    Callback m_callback;
};

} // namespace caffa
