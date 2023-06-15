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
#include "cafFieldJsonCapability.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectPerformer.h"

#include "cafFieldHandle.h"

#include <nlohmann/json.hpp>

#include <iomanip>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::readObjectFromJson( ObjectHandle* object, const nlohmann::json& jsonObject ) const
{
    CAFFA_TRACE( "Reading fields from json with serialize setting: writeTypesAndValidators = "
                 << this->writeTypesAndValidators() << ", serializeDataTypes = " << this->serializeDataTypes()
                 << ", serializeUuids = " << this->serializeUuids() );

    CAFFA_ASSERT( jsonObject.is_object() );

    for ( const auto& [keyword, value] : jsonObject.items() )
    {
        CAFFA_TRACE( "Reading field: " << keyword << " with value " << value.dump() );

        if ( keyword == "uuid" && this->serializeUuids() )
        {
            CAFFA_TRACE( "Found UUID: " << value );
            const auto& uuid = value;
            CAFFA_ASSERT( uuid.is_string() && !uuid.empty() );
            object->setUuid( uuid );
        }
        else if ( keyword == "keyword" || keyword == "class" )
        {
            const auto& classKeyword = value;
            CAFFA_ASSERT(
                classKeyword.is_string() &&
                ObjectHandle::matchesClassKeyword( classKeyword.get<std::string>(), object->classInheritanceStack() ) );
        }
        else if ( this->writeTypesAndValidators() && !value.is_null() )
        {
            auto fieldHandle = object->findField( keyword );
            if ( fieldHandle && fieldHandle->capability<FieldJsonCapability>() )
            {
                if ( this->fieldSelector() && !this->fieldSelector()( fieldHandle ) ) continue;

                auto ioFieldHandle = fieldHandle->capability<FieldJsonCapability>();
                if ( ioFieldHandle )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::writeObjectToJson( const ObjectHandle* object, nlohmann::json& jsonObject ) const
{
    CAFFA_TRACE( "Writing fields from json with serialize setting: writeTypesAndValidators = "
                 << this->writeTypesAndValidators() << ", serializeDataTypes = " << this->serializeDataTypes()
                 << ", serializeUuids = " << this->serializeUuids() );
    jsonObject["keyword"] = object->classKeyword();

    if ( this->serializeUuids() )
    {
        if ( !object->uuid().empty() )
        {
            CAFFA_TRACE( "Writing UUID: " << object->uuid() );
            jsonObject["uuid"] = object->uuid();
        }
    }

    for ( auto field : object->fields() )
    {
        if ( this->fieldSelector() && !this->fieldSelector()( field ) ) continue;

        auto keyword = field->keyword();

        const FieldJsonCapability* ioCapability = field->capability<FieldJsonCapability>();
        if ( ioCapability && keyword != "uuid" )
        {
            nlohmann::json value;
            ioCapability->writeToJson( value, *this );
            jsonObject[keyword] = value;
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
        auto keyword_it = jsonValue.find( "keyword" );
        if ( keyword_it == jsonValue.end() )
        {
            keyword_it = jsonValue.find( "class" );
        }
        auto uuid_it = jsonValue.find( "uuid" );
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
    readObjectFromJson( object, jsonValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string JsonSerializer::writeObjectToString( const ObjectHandle* object ) const
{
    nlohmann::json jsonObject = nlohmann::json::object();
    writeObjectToJson( object, jsonObject );
    return jsonObject.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr JsonSerializer::copyBySerialization( const ObjectHandle* object ) const
{
    std::string string = writeObjectToString( object );

    ObjectHandle::Ptr objectCopy = createObjectFromString( string );
    if ( !objectCopy ) return nullptr;

    ObjectPerformer<> performer( []( ObjectHandle* object ) { object->initAfterRead(); } );
    performer.visitObject( objectCopy.get() );

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr JsonSerializer::copyAndCastBySerialization( const ObjectHandle* object,
                                                              const std::string&  destinationClassKeyword ) const
{
    std::string string = writeObjectToString( object );

    ObjectHandle::Ptr objectCopy = m_objectFactory->create( destinationClassKeyword );

    bool sourceInheritsDestination =
        ObjectHandle::matchesClassKeyword( destinationClassKeyword, object->classInheritanceStack() );
    bool destinationInheritsSource =
        ObjectHandle::matchesClassKeyword( object->classKeyword(), objectCopy->classInheritanceStack() );

    if ( !sourceInheritsDestination && !destinationInheritsSource ) return nullptr;

    nlohmann::json jsonObject = nlohmann::json::parse( string );
    readObjectFromJson( objectCopy.get(), jsonObject );

    ObjectPerformer<> performer( []( ObjectHandle* object ) { object->initAfterRead(); } );
    performer.visitObject( objectCopy.get() );

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr JsonSerializer::createObjectFromString( const std::string& string ) const
{
    CAFFA_INFO( string );
    if ( string.empty() ) return nullptr;

    nlohmann::json jsonObject = nlohmann::json::parse( string );

    if ( jsonObject.is_null() ) return nullptr;

    auto classNameElement = jsonObject.find( "keyword" );
    if ( classNameElement == jsonObject.end() )
    {
        classNameElement = jsonObject.find( "class" );
    }

    if ( classNameElement == jsonObject.end() ) return nullptr;

    const auto& jsonClassKeyword = *classNameElement;

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    std::string classKeyword = jsonClassKeyword.get<std::string>();

    ObjectHandle::Ptr newObject = m_objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readObjectFromJson( newObject.get(), jsonObject );

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

    readObjectFromJson( object, document );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void JsonSerializer::writeStream( const ObjectHandle* object, std::ostream& file ) const
{
    nlohmann::json document = nlohmann::json::object();
    writeObjectToJson( object, document );

    file << std::setw( 2 ) << document;
}
