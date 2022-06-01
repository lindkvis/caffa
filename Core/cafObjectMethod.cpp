//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafObjectMethod.h"

using namespace caffa;

CAFFA_SOURCE_INIT( ObjectMethodResult, "ObjectMethodResult", "Object" )
CAFFA_ABSTRACT_SOURCE_INIT( ObjectMethod, "ObjectMethod", "Object" )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethodResult::ObjectMethodResult( bool retValue, const std::string& errMsg )
{
    initField( status, "status" ).withDefault( retValue );
    initField( errorMessage, "error_message" ).withDefault( errMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethod::ObjectMethod( caffa::not_null<ObjectHandle*> self )
    : m_self( self )
{
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
std::unique_ptr<ObjectMethod> ObjectMethodFactory::createMethod( caffa::not_null<ObjectHandle*> self,
                                                                 const std::string&             methodName )
{
    auto classNames = self->capability<ObjectIoCapability>()->classInheritanceStack();
    for ( auto className : classNames )
    {
        auto classIt = m_factoryMap.find( className );
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

    auto classNames = self->capability<ObjectIoCapability>()->classInheritanceStack();
    for ( auto className : classNames )
    {
        auto classIt = m_factoryMap.find( className );
        if ( classIt != m_factoryMap.end() )
        {
            for ( auto methodPair : classIt->second )
            {
                methods.push_back( methodPair.first );
            }
        }
    }
    return methods;
}
