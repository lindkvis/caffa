//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
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
#pragma once

#include "cafDefaultObjectFactory.h"
#include "cafFieldHandle.h"

#include <functional>
#include <iostream>
#include <memory>

namespace caffa
{
class ObjectIoCapability;
class ObjectHandle;
class ObjectFactory;

//==================================================================================================
//
//
//
//==================================================================================================
class ObjectSerializer
{
public:
    using FieldSelector = std::function<bool( FieldHandle* )>;

    ObjectSerializer( bool           copyDataValues,
                      ObjectFactory* objectFactory = DefaultObjectFactory::instance(),
                      FieldSelector  fieldSelector = nullptr )
        : m_copyDataValues( copyDataValues )
        , m_objectFactory( objectFactory )
        , m_fieldSelector( fieldSelector )
    {
    }

    /// Convenience methods to serialize/de-serialize this particular object (with children)
    virtual std::pair<std::string, std::string>
                        readClassKeywordAndUUIDFromObjectString( const std::string& string ) const                  = 0;
    virtual void        readObjectFromString( ObjectHandle* object, const std::string& string ) const               = 0;
    virtual std::string writeObjectToString( const ObjectHandle* object ) const                                     = 0;
    virtual std::unique_ptr<ObjectHandle> copyBySerialization( const ObjectHandle* object ) const                   = 0;
    virtual std::unique_ptr<ObjectHandle> copyAndCastBySerialization( const ObjectHandle* object,
                                                                      const std::string&  destinationClassKeyword,
                                                                      const std::string& sourceClassKeyword ) const = 0;

    virtual std::unique_ptr<ObjectHandle> createObjectFromString( const std::string& string ) const = 0;

    virtual void readStream( ObjectHandle* object, std::istream& stream ) const        = 0;
    virtual void writeStream( const ObjectHandle* object, std::ostream& stream ) const = 0;

protected:
    bool           m_copyDataValues;
    ObjectFactory* m_objectFactory;
    FieldSelector  m_fieldSelector;
};

} // End of namespace caffa
