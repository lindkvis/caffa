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

#include "cafObjectHandle.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <fstream>
#include <set>
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
    JsonSerializer( ObjectFactory* objectFactory = nullptr );
    /**
     * Clone the object by serializing to and from text string
     *
     * @param object The object to copy
     * @param destinationClassKeyword The class of the object to create.
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
     * @param serializationType
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
    ObjectFactory* objectFactory() const;

    /**
     * Get the field selector
     * @return field selector
     */
    FieldSelector fieldSelector() const;

    /**
     * Check which type of serialization we're doing
     * @return The type of serialization to do
     */
    SerializationType serializationType() const;

    /**
     * Check if we're meant to serialize UUIDs. UUIDs are used for dynamic connection to runtime objects,
     * not for writing to file. Only makes a difference when serializing data.
     * @return true if we should write the UUIDs
     */
    bool serializeUuids() const;

    JsonSerializer& setClient( bool client );
    bool            isClient() const;

    /**
     * Convenience method for reading the class keyword and uuid from a json string.
     * This is used to extract the necessary information to find the object in the object hierarchy.
     * @param string The JSON text string containing the object
     * @return pair of keyword and uuid in that order.
     */
    std::string readUUIDFromObjectString( const std::string& string ) const;

    /**
     * Convenience method to read this particular object (with children) from a json string
     * @param object ObjectHandle to read in to.
     * @param string The JSON text string containing the object
     */
    void readObjectFromString( ObjectHandle* object, const std::string& string ) const;

    /**
     * Write an object to JSON text string
     * @param object The object handle to write to string.
     * @return A JSON text string
     */
    std::string writeObjectToString( const ObjectHandle* object ) const;

    /**
     * Copy the object by serializing to text string and reading in again
     * @param object The object to copy
     * @return unique ptr containing a new copy
     */
    std::shared_ptr<ObjectHandle> copyBySerialization( const ObjectHandle* object ) const;

    /**
     * Copy the object by serializing to text string but cast to a different class keyword.
     * Note, it is still returned as a base class pointer.
     *
     * @param object The object to copy
     * @param destinationClassKeyword The class of the object to create.
     * @return unique ptr containing a new copy
     */
    std::shared_ptr<ObjectHandle> copyAndCastBySerialization( const ObjectHandle* object,
                                                              const std::string&  destinationClassKeyword ) const;

    /**
     * Create a new object from a JSON text string
     * @param string The JSON text string
     * @return unique ptr to new object
     */
    std::shared_ptr<ObjectHandle> createObjectFromString( const std::string& string ) const;

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
     */
    void writeStream( const ObjectHandle* object, std::ostream& stream ) const;

    void readObjectFromJson( ObjectHandle* object, const nlohmann::json& jsonObject ) const;
    void writeObjectToJson( const ObjectHandle* object, nlohmann::json& jsonObject ) const;

protected:
    bool           m_client;
    ObjectFactory* m_objectFactory;
    FieldSelector  m_fieldSelector;

    SerializationType m_serializationType;
    bool              m_serializeUuids;
};

} // End of namespace caffa

/**
 * Serialiser for chrono time points as regular int64_t nanoseconds since epoch
 */
namespace nlohmann
{
template <typename Clock, typename Duration>
struct adl_serializer<std::chrono::time_point<Clock, Duration>>
{
    static void to_json( json& j, const std::chrono::time_point<Clock, Duration>& tp )
    {
        j = std::chrono::duration_cast<Duration>( tp.time_since_epoch() ).count();
    }

    static void from_json( const json& j, std::chrono::time_point<Clock, Duration>& tp )
    {
        Duration duration( j.get<int64_t>() );
        tp = std::chrono::steady_clock::time_point( duration );
    }
};

/**
 * Serialiser for chrono durations
 */
template <typename IntType, typename Period>
struct adl_serializer<std::chrono::duration<IntType, Period>>
{
    static void to_json( json& j, const std::chrono::duration<IntType, Period>& duration )
    {
        j = static_cast<IntType>( duration.count() );
    }

    static void from_json( const json& j, std::chrono::duration<IntType, Period>& duration )
    {
        duration = std::chrono::duration<IntType, Period>( j.get<IntType>() );
    }
};

} // namespace nlohmann
