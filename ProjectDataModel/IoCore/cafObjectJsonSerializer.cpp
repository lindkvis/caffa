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
#include "cafObjectJsonSerializer.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafFieldIoCapability.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include "cafFieldHandle.h"

#include <nlohmann/json.hpp>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void readFieldsFromJson( ObjectHandle*                   object,
                         const nlohmann::json&           jsonObject,
                         ObjectFactory*                  objectFactory,
                         bool                            copyDataValues,
                         ObjectSerializer::FieldSelector fieldSelector )
{
    CAFFA_TRACE( "Reading fields from json with copyDataValues: " << copyDataValues );
    if ( fieldSelector )
    {
        CAFFA_TRACE( "Have field selector!" );
    }

    CAFFA_ASSERT( jsonObject.is_object() );
    const auto& classKeyword = jsonObject["classKeyword"];

    CAFFA_ASSERT( classKeyword.is_string() &&
                  classKeyword.get<std::string>() == object->capability<ObjectIoCapability>()->classKeyword() );

    const auto& uuid = jsonObject["uuid"];
    CAFFA_ASSERT( uuid.is_string() && !uuid.empty() );
    object->setUuid( uuid );

    if ( !copyDataValues ) return;

    if ( !jsonObject.contains( "fields" ) )
    {
        CAFFA_DEBUG( "Object has no fields" );
    }

    auto jsonFields = jsonObject["fields"];

    for ( const nlohmann::json& field : jsonFields )
    {
        CAFFA_TRACE( "Reading field: " << field.dump() );
        auto keyIt   = field.find( "keyword" );
        auto valueIt = field.find( "value" );
        if ( keyIt == field.end() )
        {
            CAFFA_WARNING( "Could not find keyword in " << field.dump() );
            continue;
        }

        if ( valueIt == field.end() || valueIt->is_null() )
        {
            continue;
        }

        auto fieldHandle = object->findField( *keyIt );
        if ( fieldHandle && fieldHandle->capability<FieldIoCapability>() )
        {
            if ( fieldSelector && !fieldSelector( fieldHandle ) ) continue;

            auto ioFieldHandle = fieldHandle->capability<FieldIoCapability>();
            if ( ioFieldHandle && ioFieldHandle->isIOReadable() )
            {
                ioFieldHandle->readFromJson( *valueIt, objectFactory, copyDataValues );
            }
            else
            {
                CAFFA_WARNING( "field handle not readable" );
            }
        }
        else
        {
            CAFFA_WARNING( "Could not find field " << *keyIt );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void writeFieldsToJson( const ObjectHandle*             object,
                        nlohmann::json&                 jsonObject,
                        bool                            copyDataValues,
                        ObjectSerializer::FieldSelector fieldSelector )
{
    std::string classKeyword = object->capability<ObjectIoCapability>()->classKeyword();
    CAFFA_ASSERT( ObjectIoCapability::isValidElementName( classKeyword ) );
    jsonObject["classKeyword"] = classKeyword;
    jsonObject["uuid"]         = object->uuid();

    nlohmann::json jsonFields = nlohmann::json::array();

    for ( auto field : object->fields() )
    {
        if ( fieldSelector && !fieldSelector( field ) ) continue;

        std::string keyword = field->keyword();

        const FieldIoCapability* ioCapability = field->capability<FieldIoCapability>();
        if ( ioCapability && ( ioCapability->isIOWritable() || ioCapability->isIOReadable() ) && keyword != "uuid" )
        {
            CAFFA_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

            nlohmann::json jsonField;
            jsonField["keyword"] = keyword;
            jsonField["type"]    = field->dataType();

            if ( copyDataValues && ioCapability->isIOReadable() )
            {
                nlohmann::json value;
                ioCapability->writeToJson( value, copyDataValues );
                jsonField["value"] = value;
                CAFFA_TRACE( "Writing field " << keyword << "(" << field->dataType() << ") = " << value.dump() );
            }
            else
            {
                CAFFA_TRACE( "Writing field " << keyword << "(" << field->dataType() << ") to json without value " );
            }
            jsonFields.push_back( jsonField );
        }
    }
    jsonObject["fields"] = jsonFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectJsonSerializer::ObjectJsonSerializer( bool copyDataValues, ObjectFactory* objectFactory, FieldSelector fieldSelector )
    : ObjectSerializer( copyDataValues, objectFactory, fieldSelector )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string>
    ObjectJsonSerializer::readClassKeywordAndUUIDFromObjectString( const std::string& string ) const
{
    nlohmann::json jsonValue = nlohmann::json::parse( string );

    std::pair<std::string, std::string> keywordUuidPair;

    if ( jsonValue.is_object() )
    {
        auto keyword_it = jsonValue.find( "classKeyword" );
        auto uuid_it    = jsonValue.find( "uuid" );
        if ( keyword_it != jsonValue.end() ) keywordUuidPair.first = keyword_it.value();
        if ( uuid_it != jsonValue.end() ) keywordUuidPair.second = uuid_it.value();
    }
    return keywordUuidPair;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonSerializer::readObjectFromString( ObjectHandle* object, const std::string& string ) const
{
    nlohmann::json jsonValue = nlohmann::json::parse( string );
    readFieldsFromJson( object, jsonValue, m_objectFactory, true, m_fieldSelector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectJsonSerializer::writeObjectToString( const ObjectHandle* object ) const
{
    nlohmann::json jsonObject = nlohmann::json::object();
    writeFieldsToJson( object, jsonObject, m_copyDataValues, m_fieldSelector );
    return jsonObject.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectJsonSerializer::copyBySerialization( const ObjectHandle* object ) const
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
std::unique_ptr<ObjectHandle>
    ObjectJsonSerializer::copyAndCastBySerialization( const ObjectHandle* object,
                                                      const std::string&  destinationClassKeyword ) const
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
    readFieldsFromJson( objectCopy.get(), jsonObject, m_objectFactory, true, m_fieldSelector );
    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectJsonSerializer::createObjectFromString( const std::string& string ) const
{
    CAFFA_TRACE( string );
    if ( string.empty() ) return nullptr;

    nlohmann::json jsonObject       = nlohmann::json::parse( string );
    const auto&    jsonClassKeyword = jsonObject["classKeyword"];

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    std::string classKeyword = jsonClassKeyword.get<std::string>();

    std::unique_ptr<ObjectHandle> newObject = m_objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFieldsFromJson( newObject.get(), jsonObject, m_objectFactory, m_copyDataValues, m_fieldSelector );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonSerializer::readStream( ObjectHandle* object, std::istream& file ) const
{
    CAFFA_ASSERT( file );

    nlohmann::json document;
    file >> document;

    readFieldsFromJson( object, document, m_objectFactory, m_copyDataValues, m_fieldSelector );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonSerializer::writeStream( const ObjectHandle* object, std::ostream& file ) const
{
    nlohmann::json document;
    writeFieldsToJson( object, document, m_copyDataValues, m_fieldSelector );

    file << document;
}
