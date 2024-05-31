// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2020- Kontur AS
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

#include "cafDataFieldAccessor.h"

#include "cafField.h"
#include "cafFieldDocumentationCapability.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafFieldValidator.h"

#include <memory>

namespace caffa
{
template <typename T>
concept DerivesFromFieldHandle = std::is_base_of<FieldHandle, T>::value;

/**
 * Helper class that is initialised with Object::initField and allows
 * .. addding additional features to the field.
 */
template <DerivesFromFieldHandle FieldType>
class FieldInitHelper
{
public:
    using GetMethod = std::function<typename FieldType::FieldDataType()>;
    using SetMethod = std::function<void( const typename FieldType::FieldDataType& )>;

    FieldInitHelper( FieldType& field, const std::string& keyword )
        : m_field( field )
        , m_keyword( keyword )
    {
    }

    FieldInitHelper& withDefault( const typename FieldType::FieldDataType& defaultValue )
    {
        m_field.setDefaultValue( defaultValue );
        m_field = defaultValue;
        return *this;
    }

    FieldInitHelper& withScripting( const std::string& scriptFieldKeyword, bool readable = true, bool writable = true )
    {
        m_field.addCapability(
            std::make_unique<FieldScriptingCapability>( scriptFieldKeyword.empty() ? m_keyword : scriptFieldKeyword,
                                                        readable,
                                                        writable ) );
        return *this;
    }

    FieldInitHelper& withScripting( bool readable = true, bool writable = true )
    {
        m_field.addCapability( std::make_unique<FieldScriptingCapability>( m_keyword, readable, writable ) );
        return *this;
    }

    FieldInitHelper& withAccessor( std::unique_ptr<DataFieldAccessor<typename FieldType::FieldDataType>> accessor )
    {
        m_field.setAccessor( std::move( accessor ) );
        return *this;
    }

    FieldInitHelper& withProxyGetAccessor( GetMethod getMethod )
    {
        auto accessor = std::make_unique<caffa::FieldProxyAccessor<typename FieldType::FieldDataType>>();
        accessor->registerGetMethod( getMethod );
        return withAccessor( std::move( accessor ) );
    }

    FieldInitHelper& withProxySetAccessor( SetMethod setMethod )
    {
        auto accessor = std::make_unique<caffa::FieldProxyAccessor<typename FieldType::FieldDataType>>();
        accessor->registerSetMethod( setMethod );
        return withAccessor( std::move( accessor ) );
    }

    FieldInitHelper& withProxyGetSetAccessor( GetMethod getMethod, SetMethod setMethod )
    {
        auto accessor = std::make_unique<caffa::FieldProxyAccessor<typename FieldType::FieldDataType>>();
        accessor->registerGetMethod( getMethod );
        accessor->registerSetMethod( setMethod );
        return withAccessor( std::move( accessor ) );
    }

    FieldInitHelper& withValidator( std::unique_ptr<FieldValidator<typename FieldType::FieldDataType>> validator )
    {
        m_field.addValidator( std::move( validator ) );
        return *this;
    }

    FieldInitHelper& withDoc( const std::string& documentation )
    {
        auto doc = std::make_unique<caffa::FieldDocumentationCapability>( documentation );
        m_field.addCapability( std::move( doc ) );
        return *this;
    }

    FieldInitHelper& markDeprecated()
    {
        m_field.markDeprecated();
        return *this;
    }

private:
    FieldInitHelper()                         = delete;
    FieldInitHelper( const FieldInitHelper& ) = delete;
    FieldInitHelper( FieldInitHelper&& )      = delete;

    FieldInitHelper& operator=( const FieldInitHelper& ) = delete;
    FieldInitHelper& operator=( FieldInitHelper&& )      = delete;

    FieldType&         m_field;
    const std::string& m_keyword;
};
} // namespace caffa