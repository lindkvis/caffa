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

#include "cafField.h"
#include "cafFieldInitHelper.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafMethodInitHelper.h"
#include "cafObjectHandle.h"
#include "cafObjectMacros.h"

namespace caffa
{
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
        addField( field, keyword );
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
        addMethod( method, keyword );

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

    /**
     * @brief Get the Object UUID. Used to associate server and client objects.
     *
     * @return std::string
     */
    std::string uuid() const override;

    /**
     * @brief Set the UUID for the Object
     *
     * @param uuid
     */
    void setUuid( const std::string& uuid ) override;

private:
    caffa::Field<std::string> m_uuid;
};

} // End of namespace caffa
