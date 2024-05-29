// ##################################################################################################
//
//    CAFFA
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
// ##################################################################################################
#pragma once

#include "cafMethod.h"

#include <concepts>
#include <memory>

namespace caffa
{

template <typename MethodType>
concept DerivesFromMethodHandle = std::is_base_of<MethodHandle, MethodType>::value;
/**
 * Helper class that is initialised with Object::initMethod and allows
 * .. addding additional features to the Method.
 */
template <DerivesFromMethodHandle MethodType>
class MethodInitHelper
{
public:
    MethodInitHelper( MethodType& method, const std::string& keyword )
        : m_method( method )
        , m_keyword( keyword )
    {
    }

    /**
     * @brief Apply documentation.
     *
     * @param documentation
     * @return MethodInitHelper&
     */
    MethodInitHelper& withDoc( const std::string& documentation )
    {
        m_method.setDocumentation( documentation );
        return *this;
    }

    /**
     * @brief Set argument names. Required for positional arguments
     *
     * @param argumentNames A list of argument names applied positionally.
     * @return MethodInitHelper&
     */
    MethodInitHelper& withArgumentNames( const std::vector<std::string>& argumentNames )
    {
        m_method.setArgumentNames( argumentNames );
        return *this;
    }

    /**
     * @brief Make the method a const method that cannot alter values
     * Analoguous to a regular const object method.
     *
     * @return MethodInitHelper&
     */
    MethodInitHelper& makeConst()
    {
        m_method.setConst( true );
        return *this;
    }

private:
    MethodInitHelper()                          = delete;
    MethodInitHelper( const MethodInitHelper& ) = delete;
    MethodInitHelper( MethodInitHelper&& )      = delete;

    MethodInitHelper& operator=( const MethodInitHelper& ) = delete;
    MethodInitHelper& operator=( MethodInitHelper&& )      = delete;

    MethodType&        m_method;
    const std::string& m_keyword;
};
} // namespace caffa