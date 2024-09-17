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
#include "cafJsonSerializer.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafFieldIoCapability.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectPerformer.h"

#include "cafFieldHandle.h"

#include <boost/json.hpp>

#include <iomanip>
#include <set>
#include <utility>

using namespace caffa;

std::string JsonSerializer::serializationTypeLabel( SerializationType type )
{
    switch ( type )
    {
        case SerializationType::DATA_FULL:
            return "DATA";
        case SerializationType::DATA_SKELETON:
            return "SKELETON";
        case SerializationType::SCHEMA:
            return "SCHEMA";
        case SerializationType::PATH:
            return "PATH";
    }
    CAFFA_ASSERT( false );
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
JsonSerializer::JsonSerializer( ObjectFactory* objectFactory /* = nullptr */ )
    : m_client( false )
    , m_objectFactory( objectFactory == nullptr ? DefaultObjectFactory::instance().get() : objectFactory )
    , m_serializationType( SerializationType::DATA_FULL )
    , m_serializeUuids( true )
    , m_level( -1 )
{
}

JsonSerializer& JsonSerializer::setFieldSelector( FieldSelector fieldSelector )
{
    m_fieldSelector = std::move( fieldSelector );
    return *this;
}

JsonSerializer& JsonSerializer::setSerializationType( SerializationType type )
{
    m_serializationType = type;
    return *this;
}

JsonSerializer& JsonSerializer::setSerializeUuids( bool serializeUuids )
{
    m_serializeUuids = serializeUuids;
    return *this;
}

ObjectFactory* JsonSerializer::objectFactory() const
{
    return m_objectFactory;
}

JsonSerializer::FieldSelector JsonSerializer::fieldSelector() const
{
    return m_fieldSelector;
}

JsonSerializer::SerializationType JsonSerializer::serializationType() const
{
    return m_serializationType;
}

bool JsonSerializer::serializeUuids() const
{
    return m_serializeUuids;
}

JsonSerializer& JsonSerializer::setClient( bool client )
{
    m_client = client;
    return *this;
}

bool JsonSerializer::isClient() const
{
    return m_client;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readObjectFromJson( ObjectHandle* object, const json::object& jsonObject ) const
{
    CAFFA_TRACE( "Reading fields on " << ( isClient() ? "client" : "server" )
                                      << " from json with type = " << serializationTypeLabel( this->serializationType() )
                                      << ", serializeUuids = " << this->serializeUuids() );

    if ( this->serializationType() != SerializationType::DATA_FULL &&
         this->serializationType() != SerializationType::DATA_SKELETON )
    {
        CAFFA_ERROR( "Reading JSON into objects only makes sense for data" );
        return;
    }

    ++m_level;

    if ( this->serializeUuids() )
    {
        if ( auto it = jsonObject.find( "uuid" ); it != jsonObject.end() )
        {
            object->setUuid( json::from_json<std::string>( it->value() ) );
        }
    }

    if ( this->serializationType() == SerializationType::DATA_SKELETON )
    {
        CAFFA_ASSERT( this->serializeUuids() && "Does not make sense to serialise data skeleton without UUIDs" );
        CAFFA_ASSERT( jsonObject.contains( "uuid" ) );
    }

    for ( const auto& [keyword, value] : jsonObject )
    {
        CAFFA_TRACE( "Reading field: " << keyword << " with value " << json::dump( value ) );

        if ( keyword == "uuid" || keyword == "$id" )
        {
            continue;
        }
        if ( keyword == "keyword" || keyword == "class" )
        {
            const auto& classKeyword = value;
            CAFFA_ASSERT( classKeyword.is_string() &&
                          ObjectHandle::matchesClassKeyword( json::from_json<std::string>( classKeyword ),
                                                             object->classInheritanceStack() ) );
        }
        else if ( this->serializationType() == SerializationType::DATA_FULL && !value.is_null() && keyword != "methods" )
        {
            if ( auto fieldHandle = object->findField( keyword );
                 fieldHandle && fieldHandle->capability<FieldIoCapability>() && fieldHandle->isWritable() )
            {
                if ( this->fieldSelector() && !this->fieldSelector()( fieldHandle ) ) continue;

                if ( auto ioFieldHandle = fieldHandle->capability<FieldIoCapability>(); ioFieldHandle )
                {
                    ioFieldHandle->readFromJson( value, *this );
                }
                else
                {
                    CAFFA_WARNING( "field handle not readable" );
                }
            }
            else
            {
                CAFFA_WARNING( "Could not find field " << keyword );
            }
        }
    }

    --m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::writeObjectToJson( const ObjectHandle* object, json::object& jsonObject ) const
{
    ++m_level;
    CAFFA_TRACE( "Writing fields for "
                 << object->classKeyword() << " -> " << ( isClient() ? "client" : "server" )
                 << " from json with serialize setting: type = " << serializationTypeLabel( this->serializationType() )
                 << ", serializeUuids = " << this->serializeUuids() << ", level: " << m_level );

    if ( this->serializationType() == SerializationType::SCHEMA )
    {
        std::set<std::string> parentalFields;
        std::set<std::string> parentalMethods;

        auto inheritanceStack = object->classInheritanceStack();

        std::shared_ptr<caffa::ObjectHandle> parentClassInstance;
        for ( auto it = inheritanceStack.begin() + 1; it != inheritanceStack.end(); ++it )
        {
            parentClassInstance = DefaultObjectFactory::instance()->create( *it );
            if ( parentClassInstance ) break;
        }

        std::string parentClassKeyword;
        if ( parentClassInstance )
        {
            for ( auto field : parentClassInstance->fields() )
            {
                parentalFields.insert( field->keyword() );
            }
            for ( auto method : parentClassInstance->methods() )
            {
                parentalMethods.insert( method->keyword() );
            }
            parentClassKeyword = parentClassInstance->classKeyword();
        }

        auto jsonClass = boost::json::object();

        jsonClass["type"]   = "object";
        auto jsonProperties = boost::json::object();

        jsonProperties["keyword"] = { { "type", "string" } };
        jsonProperties["uuid"]    = { { "type", "string" } };
        //  jsonObject["title"]       = object->classKeyword();

        if ( !object->classDocumentation().empty() )
        {
            jsonClass["description"] = object->classDocumentation();
        }

        for ( auto field : object->fields() )
        {
            if ( this->fieldSelector() && !this->fieldSelector()( field ) ) continue;

            if ( field->isDeprecated() ) continue;

            auto keyword = field->keyword();
            if ( parentalFields.contains( keyword ) ) continue;

            const FieldIoCapability* ioCapability = field->capability<FieldIoCapability>();
            if ( ioCapability && ( field->isReadable() || field->isWritable() ) )
            {
                json::value value;
                ioCapability->writeToJson( value, *this );
                jsonProperties[keyword] = value;
            }
        }

        auto methods = json::object();
        for ( auto method : object->methods() )
        {
            auto keyword = method->keyword();
            if ( parentalMethods.contains( keyword ) ) continue;

            methods[keyword] = json::parse( method->schema() );
        }
        if ( !methods.empty() )
        {
            auto methodsObject          = json::object();
            methodsObject["type"]       = "object";
            methodsObject["properties"] = methods;
            jsonProperties["methods"]   = methodsObject;
        }

        jsonClass["properties"] = jsonProperties;
        jsonClass["required"]   = { "keyword", "uuid" };

        if ( parentClassInstance )
        {
            auto         jsonAllOf    = json::array();
            json::object objectSchema = { { "$ref", "#/components/object_schemas/" + parentClassKeyword } };
            jsonAllOf.push_back( objectSchema );

            jsonAllOf.push_back( jsonClass );
            jsonObject["allOf"] = jsonAllOf;
        }
        else
        {
            jsonObject = jsonClass;
        }
        jsonObject["$schema"] = "https://json-schema.org/draft/2020-12/schema";
        jsonObject["$id"]     = "/openapi.json/components/object_schemas/" + std::string( object->classKeyword() );
    }
    else
    {
        jsonObject["keyword"] = object->classKeyword();
        if ( this->serializeUuids() && !object->uuid().empty() )
        {
            jsonObject["uuid"] = object->uuid();
        }

        if ( m_level == 0 || this->serializationType() != SerializationType::DATA_SKELETON )
        {
            for ( auto field : object->fields() )
            {
                if ( this->fieldSelector() && !this->fieldSelector()( field ) ) continue;

                if ( field->isDeprecated() ) continue;

                auto keyword = field->keyword();

                const FieldIoCapability* ioCapability = field->capability<FieldIoCapability>();
                if ( ioCapability && field->isReadable() )
                {
                    json::value value;
                    ioCapability->writeToJson( value, *this );
                    if ( !value.is_null() ) jsonObject[keyword] = value;
                }
            }
        }
    }
    --m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string JsonSerializer::readUUIDFromObjectString( const std::string& string )
{
    if ( const json::value jsonValue = json::parse( string ); jsonValue.is_object() )
    {
        const auto& jsonObject = jsonValue.get_object();
        if ( const auto uuid_it = jsonObject.find( "uuid" ); uuid_it != jsonObject.end() )
            return json::from_json<std::string>( uuid_it->value() );
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readObjectFromString( ObjectHandle* object, const std::string& string ) const
{
    const json::value jsonValue = json::parse( string );
    readObjectFromJson( object, jsonValue.as_object() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string JsonSerializer::writeObjectToString( const ObjectHandle* object ) const
{
    json::object jsonObject;
    writeObjectToJson( object, jsonObject );
    return json::dump( jsonObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> JsonSerializer::copyBySerialization( const ObjectHandle* object ) const
{
    const std::string string = writeObjectToString( object );

    std::shared_ptr<ObjectHandle> objectCopy = createObjectFromString( string );
    if ( !objectCopy ) return nullptr;

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> JsonSerializer::copyAndCastBySerialization( const ObjectHandle* object,
                                                                          const std::string_view& destinationClassKeyword ) const
{
    std::string string = writeObjectToString( object );

    std::shared_ptr<ObjectHandle> objectCopy = m_objectFactory->create( destinationClassKeyword );

    bool sourceInheritsDestination =
        ObjectHandle::matchesClassKeyword( destinationClassKeyword, object->classInheritanceStack() );
    bool destinationInheritsSource =
        ObjectHandle::matchesClassKeyword( object->classKeyword(), objectCopy->classInheritanceStack() );

    if ( !sourceInheritsDestination && !destinationInheritsSource ) return nullptr;

    const json::value jsonValue = json::parse( string );
    readObjectFromJson( objectCopy.get(), jsonValue.as_object() );

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> JsonSerializer::createObjectFromString( const std::string& string ) const
{
    CAFFA_TRACE( "Creating object from JSON string '" << string << "'" );

    if ( string.empty() ) return nullptr;

    const json::value jsonValue = json::parse( string );
    if ( jsonValue.is_null() ) return nullptr;

    return createObjectFromJson( jsonValue.as_object() );
}

std::shared_ptr<ObjectHandle> JsonSerializer::createObjectFromJson( const json::object& jsonObject ) const
{
    auto classNameElement = jsonObject.find( "keyword" );
    if ( classNameElement == jsonObject.end() )
    {
        classNameElement = jsonObject.find( "class" );
    }

    if ( classNameElement == jsonObject.end() ) return nullptr;

    const auto& jsonClassKeyword = classNameElement->value();

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    const auto classKeyword = json::from_json<std::string>( jsonClassKeyword );

    std::shared_ptr<ObjectHandle> newObject = m_objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readObjectFromJson( newObject.get(), jsonObject );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readStream( ObjectHandle* object, std::istream& file ) const
{
    const std::string str( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );

    readObjectFromString( object, str );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::writeStream( const ObjectHandle* object, std::ostream& file ) const
{
    json::object document;
    writeObjectToJson( object, document );

    file << json::dump( document );
}
