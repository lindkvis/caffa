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
#include "cafFieldInitHelper.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafFieldValidator.h"
#include "cafMethodInitHelper.h"
#include "cafObjectCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectMacros.h"

#include <set>

namespace caffa
{
class ObjectCapability;
class ObjectFactory;

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
     * @param callback The method that will be called locally
     */
    template <typename MethodType, typename CallbackT>
    MethodInitHelper<MethodType> initMethod( MethodType& method, const std::string& keyword, CallbackT&& callback )
    {
        method.setCallback( callback );
        addMethod( &method, keyword );

        return MethodInitHelper<MethodType>( method, keyword );
    }

    /**
     * Initialises the method with a keyword and registers it with the class
     * @param method A reference to the method
     * @param keyword The method keyword. Has to be unique within the class.
     * @param argumentNames A vector of argument names
     * @param callback The method that will be called locally
     */
    template <typename MethodType, typename CallbackT>
    MethodInitHelper<MethodType> initMethodWithSession( MethodType& method, const std::string& keyword, CallbackT&& callback )
    {
        method.setCallbackWithSession( callback );
        addMethod( &method, keyword );
        return MethodInitHelper<MethodType>( method, keyword );
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
     * @brief Deep clone and cast to the typed class using an optional object
     * y
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
