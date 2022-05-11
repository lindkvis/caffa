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
#include "cafJsonSerializer.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafFieldJsonCapability.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include "cafFieldHandle.h"

#include <nlohmann/json.hpp>

#include <iomanip>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void readFieldsFromJson( ObjectHandle* object, const nlohmann::json& jsonObject, const JsonSerializer* serializer )
{
    CAFFA_TRACE( "Reading fields from json with serialize setting: serializeDataValues = "
                 << serializer->serializeDataValues() << ", serializeSchema = " << serializer->serializeSchema()
                 << ", serializeUuids = " << serializer->serializeUuids() );

    CAFFA_ASSERT( jsonObject.is_object() );

    for ( const auto& [keyword, value] : jsonObject.items() )
    {
        CAFFA_TRACE( "Reading field: " << keyword << " with value " << value.dump() );

        if ( keyword == "UUID" && serializer->serializeUuids() )
        {
            CAFFA_TRACE( "Found UUID: " << value );
            const auto& uuid = value;
            CAFFA_ASSERT( uuid.is_string() && !uuid.empty() );
            object->setUuid( uuid );
        }
        else if ( keyword == "Class" )
        {
            const auto& classKeyword = value;

            CAFFA_ASSERT( classKeyword.is_string() &&
                          classKeyword.get<std::string>() == object->capability<ObjectIoCapability>()->classKeyword() );
        }
        else if ( serializer->serializeDataValues() && !value.is_null() )
        {
            auto fieldHandle = object->findField( keyword );
            if ( fieldHandle && fieldHandle->capability<FieldJsonCapability>() )
            {
                if ( serializer->fieldSelector() && !serializer->fieldSelector()( fieldHandle ) ) continue;

                auto ioFieldHandle = fieldHandle->capability<FieldJsonCapability>();
                if ( ioFieldHandle && fieldHandle->isReadable() )
                {
                    ioFieldHandle->readFromJson( value, *serializer );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void writeFieldsToJson( const ObjectHandle* object, nlohmann::json& jsonObject, const JsonSerializer* serializer )
{
    CAFFA_TRACE( "Writing fields from json with serialize setting: serializeDataValues = "
                 << serializer->serializeDataValues() << ", serializeSchema = " << serializer->serializeSchema()
                 << ", serializeUuids = " << serializer->serializeUuids() );
    std::string classKeyword = object->capability<ObjectIoCapability>()->classKeyword();
    CAFFA_ASSERT( ObjectIoCapability::isValidElementName( classKeyword ) );
    jsonObject["Class"] = classKeyword;

    if ( serializer->serializeUuids() )
    {
        if ( !object->uuid().empty() )
        {
            CAFFA_TRACE( "Writing UUID: " << object->uuid() );
            jsonObject["UUID"] = object->uuid();
        }
    }

    if ( serializer->serializeDataValues() || serializer->serializeSchema() )
    {
        for ( auto field : object->fields() )
        {
            if ( serializer->fieldSelector() && !serializer->fieldSelector()( field ) ) continue;
            if ( !( field->isReadable() || field->isWritable() ) ) continue;

            std::string keyword = field->keyword();

            const FieldJsonCapability* ioCapability = field->capability<FieldJsonCapability>();
            if ( ioCapability && keyword != "UUID" )
            {
                CAFFA_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

                nlohmann::json value;
                ioCapability->writeToJson( value, *serializer );
                jsonObject[keyword] = value;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
JsonSerializer::JsonSerializer( ObjectFactory* objectFactory )
    : Serializer( objectFactory )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> JsonSerializer::readClassKeywordAndUUIDFromObjectString( const std::string& string ) const
{
    nlohmann::json jsonValue = nlohmann::json::parse( string );

    std::pair<std::string, std::string> keywordUuidPair;

    if ( jsonValue.is_object() )
    {
        auto keyword_it = jsonValue.find( "Class" );
        auto uuid_it    = jsonValue.find( "UUID" );
        if ( keyword_it != jsonValue.end() ) keywordUuidPair.first = keyword_it.value();
        if ( uuid_it != jsonValue.end() ) keywordUuidPair.second = uuid_it.value();
    }
    return keywordUuidPair;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readObjectFromString( ObjectHandle* object, const std::string& string ) const
{
    nlohmann::json jsonValue = nlohmann::json::parse( string );
    readFieldsFromJson( object, jsonValue, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string JsonSerializer::writeObjectToString( const ObjectHandle* object ) const
{
    nlohmann::json jsonObject = nlohmann::json::object();
    writeFieldsToJson( object, jsonObject, this );
    return jsonObject.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> JsonSerializer::copyBySerialization( const ObjectHandle* object ) const
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    std::string string = writeObjectToString( object );

    std::unique_ptr<ObjectHandle> objectCopy = createObjectFromString( string );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> JsonSerializer::copyAndCastBySerialization( const ObjectHandle* object,
                                                                          const std::string& destinationClassKeyword ) const
{
    // Can not do this without an IO capability
    auto ioCapability = object->capability<ObjectIoCapability>();
    if ( !ioCapability ) return nullptr;

    ioCapability->setupBeforeSaveRecursively();
    std::string string = writeObjectToString( object );

    std::unique_ptr<ObjectHandle> objectCopy = m_objectFactory->create( destinationClassKeyword );

    auto copyIoCapability = objectCopy->capability<ObjectIoCapability>();
    if ( !copyIoCapability ) return nullptr;

    bool sourceInheritsDestination = ioCapability->matchesClassKeyword( destinationClassKeyword );
    bool destinationInheritsSource = copyIoCapability->matchesClassKeyword( ioCapability->classKeyword() );

    if ( !sourceInheritsDestination && !destinationInheritsSource ) return nullptr;

    nlohmann::json jsonObject = nlohmann::json::parse( string );
    readFieldsFromJson( objectCopy.get(), jsonObject, this );
    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> JsonSerializer::createObjectFromString( const std::string& string ) const
{
    CAFFA_TRACE( string );
    if ( string.empty() ) return nullptr;

    nlohmann::json jsonObject       = nlohmann::json::parse( string );
    const auto&    jsonClassKeyword = jsonObject["Class"];

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    std::string classKeyword = jsonClassKeyword.get<std::string>();

    std::unique_ptr<ObjectHandle> newObject = m_objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFieldsFromJson( newObject.get(), jsonObject, this );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readStream( ObjectHandle* object, std::istream& file ) const
{
    CAFFA_ASSERT( file );

    nlohmann::json document;
    file >> document;

    readFieldsFromJson( object, document, this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::writeStream( const ObjectHandle* object, std::ostream& file ) const
{
    nlohmann::json document = nlohmann::json::object();
    writeFieldsToJson( object, document, this );

    file << std::setw( 2 ) << document;
}
