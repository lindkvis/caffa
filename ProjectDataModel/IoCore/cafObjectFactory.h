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

#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"
#include "cafPointer.h"

#include <list>
#include <string>
#include <vector>

namespace caffa
{
//==================================================================================================
//
// Factory interface for creating CAF objects derived from ObjectHandle based on class name keyword
//
//==================================================================================================
class ObjectFactory
{
public:
    std::unique_ptr<ObjectHandle> create( const std::string& classNameKeyword )
    {
        std::unique_ptr<ObjectHandle> object = doCreate( classNameKeyword );
        if ( object ) m_objects.push_back( Pointer<ObjectHandle>( object.get() ) );
        return object;
    }

    virtual std::vector<std::string> classKeywords() const = 0;

    std::list<ObjectHandle*> objects() const
    {
        std::list<ObjectHandle*> objects;
        for ( auto object : m_objects )
        {
            if ( object.notNull() )
            {
                objects.push_back( object.p() );
            }
        }
        return objects;
    }

    template <typename T>
    std::list<T*> objectsOfType() const
    {
        std::list<T*> typedObjects;
        for ( auto object : m_objects )
        {
            T* typedObject = dynamic_cast<T*>( object.p() );
            if ( typedObject )
            {
                typedObjects.push_back( typedObject );
            }
        }
        return typedObjects;
    }

    std::list<ObjectHandle*> objectsWithClassKeyword( const std::string& classKeyword ) const
    {
        std::list<ObjectHandle*> objects;
        for ( auto object : m_objects )
        {
            if ( object.notNull() && object->capability<ObjectIoCapability>()->matchesClassKeyword( classKeyword ) )
            {
                objects.push_back( object.p() );
            }
        }
        return objects;
    }

protected:
    ObjectFactory() {}
    virtual ~ObjectFactory() {}

private:
    virtual std::unique_ptr<ObjectHandle> doCreate( const std::string& classNameKeyword ) = 0;

    std::list<Pointer<ObjectHandle>> m_objects;
};

} // End of namespace caffa
