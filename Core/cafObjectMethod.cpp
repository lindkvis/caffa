// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) Ceetron Solutions AS
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

#include "cafObjectMethod.h"

using namespace caffa;

CAFFA_SOURCE_INIT( ObjectMethodResult )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethodResult::ObjectMethodResult( bool retValue, const std::string& errMsg )
    : Object( false ) // Don't generate UUID. The object method result should never be persistent.
{
    initField( status, "status" ).withDefault( retValue );
    initField( errorMessage, "error_message" ).withDefault( errMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethod::ObjectMethod( caffa::not_null<ObjectHandle*> self, Type type )
    : Object( false )
    , m_self( self )
    , m_type( type )
{
}

ObjectMethod::Type ObjectMethod::type() const
{
    return m_type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethodFactory* ObjectMethodFactory::instance()
{
    static ObjectMethodFactory* factory = new ObjectMethodFactory;
    return factory;
}

//--------------------------------------------------------------------------------------------------
/// Check the object and the inheritance stack for the specified method name
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectMethod> ObjectMethodFactory::createMethodInstance( caffa::not_null<ObjectHandle*> self,
                                                                         const std::string_view&        methodName )
{
    auto classNames = self->classInheritanceStack();
    for ( auto className : classNames )
    {
        auto classIt = m_factoryMap.find( std::string( className ) );
        if ( classIt != m_factoryMap.end() )
        {
            auto methodIt = classIt->second.find( methodName );
            if ( methodIt != classIt->second.end() )
            {
                return methodIt->second->create( self );
            }
        }
    }
    return std::unique_ptr<ObjectMethod>();
}

//--------------------------------------------------------------------------------------------------
/// Return the methods registered for the className.
//--------------------------------------------------------------------------------------------------
std::vector<std::string> caffa::ObjectMethodFactory::registeredMethodNames( caffa::not_null<const ObjectHandle*> self ) const
{
    std::vector<std::string> methods;

    auto classNames = self->classInheritanceStack();
    for ( auto className : classNames )
    {
        auto classIt = m_factoryMap.find( std::string( className ) );
        if ( classIt != m_factoryMap.end() )
        {
            for ( const auto& [methodName, creator] : classIt->second )
            {
                methods.push_back( std::string( methodName ) );
            }
        }
    }
    return methods;
}
