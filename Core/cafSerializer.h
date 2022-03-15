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

/**
 * Interface for Serializer. Can be implemented for different types of text serialization.
 */
class Serializer
{
public:
    using FieldSelector = std::function<bool( const FieldHandle* )>;

    /**
     * Constructor
     * @param serializeSchema Should we copy the values of the fields in the serialisation?
     *                       if false only the names and data types will be copied. This is used
     *                       for on-demand data value access.
     * @param objectFactory The factory used when creating new objects. Not relevant when writing.
     * @param fieldSelector A method taking a FieldHandle pointer and returning true if that field should be serialized.
     * @param writeUuids    Write object UUIDs. UUIDs are used for dynamic connection to runtime objects.
     */
    Serializer( ObjectFactory* objectFactory );

    /**
     * Convenience method for reading the class keyword and uuid from a string.
     * This is used to extract the necessary information to find the object in the object hierarchy.
     * @param string The text string containing the object
     * @return pair of keyword and uuid in that order.
     */
    virtual std::pair<std::string, std::string>
        readClassKeywordAndUUIDFromObjectString( const std::string& string ) const = 0;

    /**
     * Convenience method to read this particular object (with children) from a json string
     * @param object ObjectHandle to read in to.
     * @param string The text string containing the object
     */
    virtual void readObjectFromString( ObjectHandle* object, const std::string& string ) const = 0;

    /**
     * Write an object to ext string
     * @param object The object handle to write to string.
     * @return A text string
     */
    virtual std::string writeObjectToString( const ObjectHandle* object ) const = 0;

    /**
     * Copy the object by serializing to text string and reading in again
     * @param object The object to copy
     * @return unique ptr containing a new copy
     */
    virtual std::unique_ptr<ObjectHandle> copyBySerialization( const ObjectHandle* object ) const = 0;

    /**
     * Copy the object by serializing to text string but cast to a different class keyword.
     * Note, it is still returned as a base class pointer.
     *
     * @param object The object to copy
     * @param destinationClassKeyword The class of the object to create.
     * @return unique ptr containing a new copy
     */
    virtual std::unique_ptr<ObjectHandle>
        copyAndCastBySerialization( const ObjectHandle* object, const std::string& destinationClassKeyword ) const = 0;

    /**
     * Create a new object from a JSON text string
     * @param string The JSON text string
     * @return unique ptr to new object
     */
    virtual std::unique_ptr<ObjectHandle> createObjectFromString( const std::string& string ) const = 0;

    /**
     * Read object from an input stream
     * @param object Pointer to object to read into
     * @param stream The input stream
     */
    virtual void readStream( ObjectHandle* object, std::istream& stream ) const = 0;

    /**
     * Write object to output stream
     * @param object Pointer to object to write
     * @param stream The output stream
     */
    virtual void writeStream( const ObjectHandle* object, std::ostream& stream ) const = 0;

    /**
     * Set Field Selector
     * Since it returns a reference to it it can be used like: Serializer(objectFactory).setFieldSelector(functor);
     *
     * @param fieldSelector
     * @return cafSerializer& reference to this
     */
    Serializer& setFieldSelector( FieldSelector fieldSelector );

    /**
     * Set whether to serialize data values.
     * Since it returns a reference to it it can be used like: Serializer(objectFactory).setSerializeDataValues(false);
     *
     * @param serializeDataValues
     * @return cafSerializer& reference to this
     */
    Serializer& setSerializeDataValues( bool serializeDataValues );

    /**
     * Set whether to serialize data types
     * Since it returns a reference to it it can be used like: Serializer(objectFactory).setSerializeSchema(false);
     *
     * @param serializeSchema
     * @return cafSerializer& reference to this
     */
    Serializer& setSerializeSchema( bool serializeSchema );

    /**
     * Set whether to serialize UUIDs
     * Since it returns a reference to it it can be used like: Serializer(objectFactory).setSerializeUuids(false);
     *
     * @param serializeUuids
     * @return cafSerializer& reference to this
     */
    Serializer& setSerializeUuids( bool serializeUuids );

    /**
     * Get the object factory
     * @return object factory
     */
    ObjectFactory* objectFactory() const;

    /**
     * Get the field selector
     * @return field selector
     */
    FieldSelector fieldSelector() const;

    /**
     * Check if we're meant to serialize data values
     * @return true if we should serialize data values
     */
    bool serializeDataValues() const;

    /**
     * Check if we're meant to serialize data types
     * @return true if we should serialize schema (labels and types)
     */
    bool serializeSchema() const;

    /**
     * Check if we're meant to serialize UUIDs. UUIDs are used for dynamic connection to runtime objects,
     * not for writing to file.
     * @return true if we should write the UUIDs
     */
    bool serializeUuids() const;

protected:
    ObjectFactory* m_objectFactory;
    FieldSelector  m_fieldSelector;

    bool m_serializeDataValues;
    bool m_serializeSchema;
    bool m_serializeUuids;
};

} // End of namespace caffa
