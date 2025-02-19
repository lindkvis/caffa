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

#include "cafJsonDefinitions.h"
#include "cafObjectHandle.h"

#include <chrono>
#include <string>

namespace caffa
{
class FieldHandle;
class ObjectFactory;

/**
 * Implementation of Serializer for JSON serialization.
 */
class JsonSerializer
{
public:
    enum class SerializationType
    {
        DATA_FULL,
        DATA_SKELETON,
        SCHEMA,
        PATH
    };

    using FieldSelector = std::function<bool( const FieldHandle* )>;

    static std::string serializationTypeLabel( SerializationType type );

public:
    /**
     * Constructor
     * @param objectFactory The factory used when creating new objects. Not relevant when writing.
     */
    explicit JsonSerializer( ObjectFactory* objectFactory = nullptr );
    /**
     * Clone the object by serializing to and from text string
     *
     * @param object The object to copy
     * @return unique ptr containing a new copy
     */
    template <DerivesFromObjectHandle ObjectType>
    std::shared_ptr<ObjectType> cloneObject( const ObjectType* object ) const
    {
        auto basePointer = copyAndCastBySerialization( object, ObjectType::classKeywordStatic() );
        return std::dynamic_pointer_cast<ObjectType>( basePointer );
    }

    /**
     * Set Field Selector
     * Since it returns a reference it can be used like: Serializer(objectFactory).setFieldSelector(functor);
     *
     * @param fieldSelector
     * @return cafSerializer& reference to this
     */
    JsonSerializer& setFieldSelector( FieldSelector fieldSelector );

    /**
     * Set what to serialize (data, schema, etc)
     * Since it returns a reference it can be used like: Serializer(objectFactory).setSerializationTypes(...);
     *
     * @param type
     * @return cafSerializer& reference to this
     */
    JsonSerializer& setSerializationType( SerializationType type );

    /**
     * Set whether to serialize UUIDs. Only makes a difference when serializing data
     * Since it returns a reference, it can be used like: Serializer(objectFactory).setSerializeUuids(false);
     *
     * @param serializeUuids
     * @return cafSerializer& reference to this
     */
    JsonSerializer& setSerializeUuids( bool serializeUuids );

    /**
     * Get the object factory
     * @return object factory
     */
    [[nodiscard]] ObjectFactory* objectFactory() const;

    /**
     * Get the field selector
     * @return field selector
     */
    [[nodiscard]] FieldSelector fieldSelector() const;

    /**
     * Check which type of serialization we're doing
     * @return The type of serialization to do
     */
    [[nodiscard]] SerializationType serializationType() const;

    /**
     * Check if we're meant to serialize UUIDs. UUIDs are used for dynamic connection to runtime objects,
     * not for writing to file. Only makes a difference when serializing data.
     * @return true if we should write the UUIDs
     */
    [[nodiscard]] bool serializeUuids() const;

    JsonSerializer&    setClient( bool client );
    [[nodiscard]] bool isClient() const;

    /**
     * Convenience method for reading the class keyword and uuid from a json string.
     * This is used to extract the necessary information to find the object in the object hierarchy.
     * @param string The JSON text string containing the object
     * @return pair of keyword and uuid in that order.
     */
    [[nodiscard]] static std::string readUUIDFromObjectString( const std::string& string );

    /**
     * Convenience method to read this particular object (with children) from a json string
     * @param object ObjectHandle to read in to.
     * @param string The JSON text string containing the object
     */
    void readObjectFromString( ObjectHandle* object, const std::string& string ) const;

    /**
     * Write an object to JSON text string
     * @param object The object handle to write to string.
     * @param pretty If true will pretty print with indentation and newlines
     * @return A JSON text string
     */
    [[nodiscard]] std::string writeObjectToString( const ObjectHandle* object, bool pretty = false ) const;

    /**
     * Copy the object by serializing to text string and reading in again
     * @param object The object to copy
     * @return unique ptr containing a new copy
     */
    [[nodiscard]] std::shared_ptr<ObjectHandle> copyBySerialization( const ObjectHandle* object ) const;

    /**
     * Copy the object by serializing to text string but cast to a different class keyword.
     * Note, it is still returned as a base class pointer.
     *
     * @param object The object to copy
     * @param destinationClassKeyword The class of the object to create.
     * @return unique ptr containing a new copy
     */
    [[nodiscard]] std::shared_ptr<ObjectHandle>
        copyAndCastBySerialization( const ObjectHandle* object, const std::string_view& destinationClassKeyword ) const;

    /**
     * Create a new object from a JSON text string
     * @param string The JSON text string
     * @return unique ptr to new object
     */
    [[nodiscard]] std::shared_ptr<ObjectHandle> createObjectFromString( const std::string& string ) const;

    /**
     * Create a new object from a JSON value
     * @param jsonValue The JSON value
     * @return unique ptr to new object
     */
    [[nodiscard]] std::shared_ptr<ObjectHandle> createObjectFromJson( const json::object& jsonValue ) const;

    /**
     * Read object from an input stream
     * @param object Pointer to object to read into
     * @param stream The input stream
     */
    void readStream( ObjectHandle* object, std::istream& stream ) const;

    /**
     * Write object to output stream
     * @param object Pointer to object to write
     * @param stream The output stream
     * @param pretty If true will pretty print with indentation and newlines
     */
    void writeStream( const ObjectHandle* object, std::ostream& stream, bool pretty = false ) const;

    void readObjectFromJson( ObjectHandle* object, const json::object& jsonValue ) const;
    void writeObjectToJson( const ObjectHandle* object, json::object& jsonValue ) const;

    void prettyPrint( std::ostream& os, json::value const& jv, std::string* indent = nullptr ) const;

protected:
    bool           m_client;
    ObjectFactory* m_objectFactory;
    FieldSelector  m_fieldSelector;

    SerializationType m_serializationType;
    bool              m_serializeUuids;

    mutable int m_level;
};

} // End of namespace caffa
