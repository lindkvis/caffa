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

CAFFA_IO_ABSTRACT_SOURCE_INIT( ObjectMethod, "ObjectMethod", "Object" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectMethod::ObjectMethod( ObjectHandle* self )
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
std::shared_ptr<ObjectMethod> ObjectMethodFactory::createMethod( ObjectHandle* self, const std::string& methodName )
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
    return std::shared_ptr<ObjectMethod>();
}

//--------------------------------------------------------------------------------------------------
/// Return the methods registered for the className. Does not investigate the inheritance stack
//--------------------------------------------------------------------------------------------------
std::vector<std::string> caffa::ObjectMethodFactory::registeredMethodNames( const std::string& className ) const
{
    std::vector<std::string> methods;

    auto classIt = m_factoryMap.find( className );
    if ( classIt != m_factoryMap.end() )
    {
        for ( auto methodPair : classIt->second )
        {
            methods.push_back( methodPair.first );
        }
    }

    return methods;
}
