//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cafDataFieldAccessor.h"
#include "cafField.h"
#include "cafFieldIoCapability.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldScriptingCapability.h"
#include "cafObjectCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafObservingPointer.h"

#include <set>

namespace caffa
{
class UiEditorAttribute;
class UiTreeOrdering;
class ObjectCapability;

#define CAFFA_HEADER_INIT CAFFA_IO_HEADER_INIT
#define CAFFA_SOURCE_INIT CAFFA_IO_SOURCE_INIT
#define CAFFA_ABSTRACT_SOURCE_INIT CAFFA_IO_ABSTRACT_SOURCE_INIT

/**
 * Helper class that is initialised with Object::initField and allows
 * .. addding additional features to the field.
 */
template <typename FieldType>
class FieldInitHelper
{
public:
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

    FieldInitHelper& withScripting( const std::string& scriptFieldKeyword = "" )
    {
        FieldScriptingCapability::addToField( &m_field, scriptFieldKeyword.empty() ? m_keyword : scriptFieldKeyword );
        return *this;
    }

    FieldInitHelper& withAccessor( std::unique_ptr<DataFieldAccessor<typename FieldType::FieldDataType>> accessor )
    {
        m_field.setAccessor( std::move( accessor ) );
        return *this;
    }

    FieldInitHelper& readOnly()
    {
        m_field.template capability<FieldIoCapability>()->setIOWritable( false );
        return *this;
    }

    FieldInitHelper& writeOnly()
    {
        m_field.template capability<FieldIoCapability>()->setIOReadable( false );
        return *this;
    }

private:
    FieldInitHelper()                         = delete;
    FieldInitHelper( const FieldInitHelper& ) = delete;
    FieldInitHelper( FieldInitHelper&& )      = delete;

    FieldInitHelper& operator=( const FieldInitHelper& ) = delete;
    FieldInitHelper& operator=( FieldInitHelper&& ) = delete;

    FieldType&         m_field;
    const std::string& m_keyword;
};
class Object : public ObjectHandle, public ObjectIoCapability
{
public:
    CAFFA_HEADER_INIT;

    Object();
    ~Object() noexcept override;

    /**
     * @brief Get the class keyword through polymorphism
     * @return a class keyword string
     */
    std::string classKeywordDynamic() const override;

    /**
     * Initialises the field with a file keyword and registers it with the class
     * including static user interface related information.
     * Note that classKeyword() is not virtual in the constructor of the Object
     * This is expected and fine.
     * @param field A reference to the field
     * @param keyword The field keyword. Has to be unique within the class.
     */
    template <typename FieldType>
    FieldInitHelper<FieldType> initField( FieldType& field, const std::string& keyword )
    {
        AddIoCapabilityToField( &field );

        addField( &field, keyword );
        return FieldInitHelper( field, keyword );
    }

    /**
     * Initialises the field with a file keyword and registers it with the class
     * including static user interface related information.
     * Note that classKeyword() is not virtual in the constructor of the Object
     * This is expected and fine.
     * This version sets a default value
     * @param field A reference to the field
     * @param keyword The field keyword. Has to be unique within the class.
     * @param defaultValue A default field value
     */
    template <typename FieldType>
    FieldInitHelper<FieldType>
        initField( FieldType& field, const std::string& keyword, const typename FieldType::FieldDataType& defaultValue )
    {
        AddIoCapabilityToField( &field );

        addField( &field, keyword );

        field.setDefaultValue( defaultValue );
        field = defaultValue;

        return FieldInitHelper( field, keyword );
    }

    std::string uuid() const override;
    void        setUuid( const std::string& uuid ) override;

private:
    Field<std::string> m_uuid;
};

} // End of namespace caffa
