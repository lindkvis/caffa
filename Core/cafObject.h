// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013-2020 Ceetron Solutions AS
//    Copyright (C) 2020- Kontur AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
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
#include "cafFieldJsonCapability.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafFieldValidator.h"
#include "cafObjectCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectMacros.h"

#include <set>

namespace caffa
{
class UiEditorAttribute;
class UiTreeOrdering;
class ObjectCapability;
class ObjectFactory;

/**
 * Helper class that is initialised with Object::initField and allows
 * .. addding additional features to the field.
 */
template <typename FieldType>
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

private:
    FieldInitHelper()                         = delete;
    FieldInitHelper( const FieldInitHelper& ) = delete;
    FieldInitHelper( FieldInitHelper&& )      = delete;

    FieldInitHelper& operator=( const FieldInitHelper& ) = delete;
    FieldInitHelper& operator=( FieldInitHelper&& )      = delete;

    FieldType&         m_field;
    const std::string& m_keyword;
};

class Object : public ObjectHandle
{
public:
    CAFFA_HEADER_INIT( Object, ObjectHandle )

    Object( bool generateUuid = true );
    ~Object() noexcept override;

    /**
     * Initialises the field with a keyword and registers it with the class
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
     * Initialises the method with a keyword and registers it with the class
     * @param method A reference to the method
     * @param keyword The method keyword. Has to be unique within the class.
     * @param argumentNames A vector of argument names
     * @param callback The method that will be called locally
     * @param type Whether it is a READ_WRITE or a READ_ONLY (const) type method
     */
    template <typename MethodType, typename CallbackT>
    void initMethod( MethodType&                     method,
                     const std::string&              keyword,
                     const std::vector<std::string>& argumentNames,
                     CallbackT&&                     callback,
                     MethodHandle::Type              type = MethodHandle::Type::READ_WRITE )
    {
        addMethod( &method, keyword, type );
        method.setCallback( callback );
        method.setArgumentNames( argumentNames );
    }

    /**
     * Initialises the method with a keyword and registers it with the class
     * @param method A reference to the method
     * @param keyword The method keyword. Has to be unique within the class.
     * @param argumentNames A vector of argument names
     * @param callback The method that will be called locally
     * @param type Whether it is a READ_WRITE or a READ_ONLY (const) type method
     */
    template <typename MethodType, typename CallbackT>
    void initMethodWithSession( MethodType&                     method,
                                const std::string&              keyword,
                                const std::vector<std::string>& argumentNames,
                                CallbackT&&                     callback,
                                MethodHandle::Type              type = MethodHandle::Type::READ_WRITE )
    {
        addMethod( &method, keyword, type );
        method.setCallbackWithSession( callback );
        method.setArgumentNames( argumentNames );
    }

    /**
     * Initialises the method with a keyword and registers it with the class
     * @param method A reference to the method
     * @param keyword The method keyword. Has to be unique within the class.
     * @param argumentNames A vector of argument names
     * @param documentation A documentation string
     * @param callback The method that will be called locally
     * @param type Whether it is a READ_WRITE or a READ_ONLY (const) type method
     */
    template <typename MethodType, typename CallbackT>
    void initMethodWithDoc( MethodType&                     method,
                            const std::string&              keyword,
                            const std::vector<std::string>& argumentNames,
                            const std::string&              documentation,
                            CallbackT&&                     callback,
                            MethodHandle::Type              type = MethodHandle::Type::READ_WRITE )
    {
        addMethod( &method, keyword, type );
        method.setCallback( callback );
        method.setArgumentNames( argumentNames );
        method.setDocumentation( documentation );
    }
    /**
     * Initialises the method with a keyword and registers it with the class
     * @param method A reference to the method
     * @param keyword The method keyword. Has to be unique within the class.
     * @param argumentNames A vector of argument names
     * @param documentation A documentation string
     * @param callback The method that will be called locally
     * @param type Whether it is a READ_WRITE or a READ_ONLY (const) type method
     */
    template <typename MethodType, typename CallbackT>
    void initMethodWithSessionAndDoc( MethodType&                     method,
                                      const std::string&              keyword,
                                      const std::vector<std::string>& argumentNames,
                                      const std::string&              documentation,
                                      CallbackT&&                     callback,
                                      MethodHandle::Type              type = MethodHandle::Type::READ_WRITE )
    {
        addMethod( &method, keyword, type );
        method.setCallbackWithSession( callback );
        method.setArgumentNames( argumentNames );
        method.setDocumentation( documentation );
    }

    std::string uuid() const override;
    void        setUuid( const std::string& uuid ) override;

    /**
     * @brief Deep clone the object using an optional object factory
     *
     * @param optionalObjectFactory if null the default object factory will be used
     * @return std::shared_ptr<Object>
     */
    ObjectHandle::Ptr deepClone( caffa::ObjectFactory* optionalObjectFactory = nullptr ) const override;

    /**
     * @brief Deep clone and cast to the typed class using an optional object factory
     *
     * @tparam DerivedClass
     * @param optionalObjectFactory if null the default object factory will be used
     * @return std::shared_ptr<DerivedClass>
     */
    template <typename DerivedClass>
    std::shared_ptr<DerivedClass> typedDeepClone( caffa::ObjectFactory* optionalObjectFactory = nullptr ) const
    {
        return std::dynamic_pointer_cast<DerivedClass>( deepClone( optionalObjectFactory ) );
    }

    /**
     * @brief Read the object content from JSON file
     * @param filePath The file path to read from
     * @return true if ok, false if not
     */
    bool readFromJsonFile( const std::string& filePath );

    /**
     * @brief Write the object content to a JSON file
     * @param filePath The file path to write to
     * @return true if ok, false if not
     */
    bool writeToJsonFile( const std::string& filePath ) const;

private:
    caffa::Field<std::string> m_uuid;
};

} // End of namespace caffa
