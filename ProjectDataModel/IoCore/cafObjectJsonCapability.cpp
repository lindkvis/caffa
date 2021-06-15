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
    readFields( object, jsonValue, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectJsonCapability::writeObjectToString( const ObjectHandle* object, bool writeServerAddress )
{
    nlohmann::json jsonObject  = nlohmann::json::object();
    jsonObject["classKeyword"] = object->capability<ObjectIoCapability>()->classKeyword();

    writeFields( object, jsonObject, writeServerAddress );

    return jsonObject.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::copyByJsonSerialization( const ObjectHandle* object, ObjectFactory* objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    std::string string = writeObjectToString( object, false );

    ObjectHandle* objectCopy = readUnknownObjectFromString( string, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                                    const std::string&  destinationClassKeyword,
                                                                    const std::string&  sourceClassKeyword,
                                                                    ObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    std::string string = writeObjectToString( object, false );

    ObjectHandle* upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    nlohmann::json jsonObject = nlohmann::json::parse( string );

    readFields( upgradedObject, jsonObject, objectFactory, true );

    upgradedObject->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::readUnknownObjectFromString( const std::string& string,
                                                                 ObjectFactory*     objectFactory,
                                                                 bool               isCopyOperation )
{
    nlohmann::json jsonObject       = nlohmann::json::parse( string );
    const auto&    jsonClassKeyword = jsonObject["classKeyword"];

    CAFFA_ASSERT( jsonClassKeyword.is_string() );
    std::string classKeyword = jsonClassKeyword.get<std::string>();

    uint64_t serverAddress = 0u;
    auto     it            = jsonObject.find( "serverAddress" );
    if ( it != jsonObject.end() )
    {
        serverAddress = it->get<uint64_t>();
    }

    ObjectHandle* newObject = objectFactory->create( classKeyword, serverAddress );

    if ( !newObject ) return nullptr;

    readFields( newObject, jsonObject, objectFactory, isCopyOperation );

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

    readFields( object, document, objectFactory ? objectFactory : DefaultObjectFactory::instance(), false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::writeFile( const ObjectHandle* object, std::ostream& file, bool writeServerAddress, bool writeValues )
{
    nlohmann::json document;
    writeFields( object, document, writeServerAddress, writeValues );

    file << document;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readFields( ObjectHandle*         object,
                                       const nlohmann::json& jsonObject,
                                       ObjectFactory*        objectFactory,
                                       bool                  isCopyOperation )
{
    CAFFA_ASSERT( jsonObject.is_object() );
    const auto& classKeyword = jsonObject["classKeyword"];

    CAFFA_ASSERT( classKeyword.is_string() &&
                  classKeyword.get<std::string>() == object->capability<ObjectIoCapability>()->classKeyword() );

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
            CAFFA_DEBUG( "Could not find keyword in " << field.dump() );
            continue;
        }
        if ( valueIt == field.end() || valueIt->is_null() )
        {
            assert( false );
            CAFFA_DEBUG( "Could not find value in " << field.dump() );
            continue;
        }

        auto fieldHandle = object->findField( *keyIt );
        if ( fieldHandle && fieldHandle->capability<FieldIoCapability>() )
        {
            auto ioFieldHandle = fieldHandle->capability<FieldIoCapability>();
            if ( !ioFieldHandle ) continue;

            bool readable = ioFieldHandle->isIOReadable();
            if ( isCopyOperation && !ioFieldHandle->isCopyable() )
            {
                readable = false;
            }
            if ( readable )
            {
                // writeToField assumes that the xmlStream points to first token of field content.
                // After reading, the xmlStream is supposed to point to the first token after the field content.
                // (typically an "endElement")
                ioFieldHandle->writeToField( *valueIt, objectFactory );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::writeFields( const ObjectHandle* object,
                                        nlohmann::json&     jsonObject,
                                        bool                writeServerAddress,
                                        bool                writeValues )
{
    std::string classKeyword = object->capability<ObjectIoCapability>()->classKeyword();
    CAFFA_ASSERT( ObjectIoCapability::isValidElementName( classKeyword ) );
    jsonObject["classKeyword"] = classKeyword;
    if ( writeServerAddress )
    {
        jsonObject["serverAddress"] = reinterpret_cast<uint64_t>( object );
    }

    nlohmann::json jsonFields = nlohmann::json::array();

    for ( auto field : object->fields() )
    {
        const FieldIoCapability* ioCapability = field->capability<FieldIoCapability>();
        if ( ioCapability && ioCapability->isIOWritable() )
        {
            std::string keyword = ioCapability->fieldHandle()->keyword();
            CAFFA_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

            nlohmann::json value;
            ioCapability->readFromField( value, writeServerAddress, writeValues );
            nlohmann::json jsonField;
            jsonField["keyword"] = keyword;
            jsonField["value"]   = value;
            jsonField["type"]    = field->dataType();

            jsonFields.push_back( jsonField );
        }
    }
    jsonObject["fields"] = jsonFields;
}
