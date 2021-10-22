#include "cafObjectJsonCapability.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include "cafFieldHandle.h"

#include <nlohmann/json.hpp>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readObjectFromString( ObjectHandle* object, const std::string& string, ObjectFactory* objectFactory )
{
    nlohmann::json jsonValue = nlohmann::json::parse( string );
    readFieldsFromJson( object, jsonValue, objectFactory, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectJsonCapability::writeObjectToString( const ObjectHandle* object )
{
    nlohmann::json jsonObject  = nlohmann::json::object();
    jsonObject["classKeyword"] = object->capability<ObjectIoCapability>()->classKeyword();

    writeFieldsToJson( object, jsonObject );

    return jsonObject.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectJsonCapability::copyByJsonSerialization( const ObjectHandle* object,
                                                                             ObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    std::string string = writeObjectToString( object );

    std::unique_ptr<ObjectHandle> objectCopy = readUnknownObjectFromString( string, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle>
    ObjectJsonCapability::copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                          const std::string&  destinationClassKeyword,
                                                          const std::string&  sourceClassKeyword,
                                                          ObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    std::string string = writeObjectToString( object );

    std::unique_ptr<ObjectHandle> upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    nlohmann::json jsonObject = nlohmann::json::parse( string );

    readFieldsFromJson( upgradedObject.get(), jsonObject, objectFactory, true );

    upgradedObject->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectJsonCapability::readUnknownObjectFromString( const std::string& string,
                                                                                 ObjectFactory*     objectFactory,
                                                                                 bool               copyDataValues )
{
    CAFFA_TRACE( string );
    if ( string.empty() ) return nullptr;

    nlohmann::json jsonObject       = nlohmann::json::parse( string );
    const auto&    jsonClassKeyword = jsonObject["classKeyword"];

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    std::string classKeyword = jsonClassKeyword.get<std::string>();

    std::unique_ptr<ObjectHandle> newObject = objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFieldsFromJson( newObject.get(), jsonObject, objectFactory, copyDataValues );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readFile( ObjectHandle* object, std::istream& file, ObjectFactory* objectFactory )
{
    CAFFA_ASSERT( file );

    nlohmann::json document;
    file >> document;

    readFieldsFromJson( object, document, objectFactory ? objectFactory : DefaultObjectFactory::instance(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::writeFile( const ObjectHandle* object, std::ostream& file, bool copyDataValues )
{
    nlohmann::json document;
    writeFieldsToJson( object, document, copyDataValues );

    file << document;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readFieldsFromJson( ObjectHandle*         object,
                                               const nlohmann::json& jsonObject,
                                               ObjectFactory*        objectFactory,
                                               bool                  copyDataValues )
{
    CAFFA_TRACE( "Reading fields from json with copyDataValues: " << copyDataValues );

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
            CAFFA_WARNING( "Could not find value in " << field.dump() );
            continue;
        }

        auto fieldHandle = object->findField( *keyIt );
        if ( fieldHandle && fieldHandle->capability<FieldIoCapability>() )
        {
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
void ObjectJsonCapability::writeFieldsToJson( const ObjectHandle* object, nlohmann::json& jsonObject, bool copyDataValues )
{
    std::string classKeyword = object->capability<ObjectIoCapability>()->classKeyword();
    CAFFA_ASSERT( ObjectIoCapability::isValidElementName( classKeyword ) );
    jsonObject["classKeyword"] = classKeyword;
    jsonObject["uuid"]         = object->uuid();

    nlohmann::json jsonFields = nlohmann::json::array();

    for ( auto field : object->fields() )
    {
        std::string keyword = field->keyword();

        const FieldIoCapability* ioCapability = field->capability<FieldIoCapability>();
        if ( ioCapability && ( ioCapability->isIOWritable() || ioCapability->isIOReadable() ) && keyword != "uuid" )
        {
            CAFFA_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

            nlohmann::json jsonField;
            jsonField["keyword"] = keyword;
            jsonField["type"]    = field->dataType();

            if ( copyDataValues )
            {
                nlohmann::json value;
                ioCapability->writeToJson( value, copyDataValues );
                jsonField["value"] = value;
            }
            jsonFields.push_back( jsonField );
        }
    }
    jsonObject["fields"] = jsonFields;
}
