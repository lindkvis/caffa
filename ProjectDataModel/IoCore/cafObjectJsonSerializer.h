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

#include "cafObjectSerializer.h"

namespace caffa
{
class ObjectHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class ObjectJsonSerializer : public ObjectSerializer
{
public:
    ObjectJsonSerializer( bool           copyDataValues,
                          ObjectFactory* objectFactory = DefaultObjectFactory::instance(),
                          FieldSelector  fieldSelector = nullptr );

    std::pair<std::string, std::string> readClassKeywordAndUUIDFromObjectString( const std::string& string ) const override;
    /// Convenience methods to serialize/de-serialize this particular object (with children)
    void        readObjectFromString( ObjectHandle* object, const std::string& string ) const override;
    std::string writeObjectToString( const ObjectHandle* object ) const override;
    std::unique_ptr<ObjectHandle> copyBySerialization( const ObjectHandle* object ) const override;
    std::unique_ptr<ObjectHandle> copyAndCastBySerialization( const ObjectHandle* object,
                                                              const std::string&  destinationClassKeyword,
                                                              const std::string&  sourceClassKeyword ) const override;

    std::unique_ptr<ObjectHandle> createObjectFromString( const std::string& string ) const override;

    void readStream( ObjectHandle* object, std::istream& stream ) const override;
    void writeStream( const ObjectHandle* object, std::ostream& stream ) const override;
};

} // End of namespace caffa
