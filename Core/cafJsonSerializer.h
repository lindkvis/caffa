// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- 3D-Radar AS
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
#pragma once

#include "cafSerializer.h"

namespace caffa
{
class ObjectHandle;

/**
 * Implementation of Serializer for JSON serialization.
 */
class JsonSerializer : public Serializer
{
public:
    /**
     * Constructor
     * @param objectFactory The factory used when creating new objects. Not relevant when writing.
     */
    JsonSerializer( ObjectFactory* objectFactory = DefaultObjectFactory::instance() );

    /**
     * Convenience method for reading the class keyword and uuid from a json string.
     * This is used to extract the necessary information to find the object in the object hierarchy.
     * @param string The JSON text string containing the object
     * @return pair of keyword and uuid in that order.
     */
    std::pair<std::string, std::string> readClassKeywordAndUUIDFromObjectString( const std::string& string ) const override;

    /**
     * Convenience method to read this particular object (with children) from a json string
     * @param object ObjectHandle to read in to.
     * @param string The JSON text string containing the object
     */
    void readObjectFromString( ObjectHandle* object, const std::string& string ) const override;

    /**
     * Write an object to JSON text string
     * @param object The object handle to write to string.
     * @return A JSON text string
     */
    std::string writeObjectToString( const ObjectHandle* object ) const override;

    /**
     * Copy the object by serializing to text string and reading in again
     * @param object The object to copy
     * @return unique ptr containing a new copy
     */
    ObjectHandle::Ptr copyBySerialization( const ObjectHandle* object ) const override;

    /**
     * Copy the object by serializing to text string but cast to a different class keyword.
     * Note, it is still returned as a base class pointer.
     *
     * @param object The object to copy
     * @param destinationClassKeyword The class of the object to create.
     * @return unique ptr containing a new copy
     */
    ObjectHandle::Ptr copyAndCastBySerialization( const ObjectHandle* object,
                                                  const std::string&  destinationClassKeyword ) const override;

    /**
     * Create a new object from a JSON text string
     * @param string The JSON text string
     * @return unique ptr to new object
     */
    ObjectHandle::Ptr createObjectFromString( const std::string& string ) const override;

    /**
     * Read object from an input stream
     * @param object Pointer to object to read into
     * @param stream The input stream
     */
    void readStream( ObjectHandle* object, std::istream& stream ) const override;

    /**
     * Write object to output stream
     * @param object Pointer to object to write
     * @param stream The output stream
     */
    void writeStream( const ObjectHandle* object, std::ostream& stream ) const override;
};
} // End of namespace caffa
