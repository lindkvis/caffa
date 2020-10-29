#include "cafObjectJsonCapability.h"

#include "cafAssert.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include "cafFieldHandle.h"

#include <QByteArray>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readObjectFromString( ObjectHandle*  object,
                                                    const QString&    string,
                                                    ObjectFactory* objectFactory )
{
    QJsonDocument document = QJsonDocument::fromJson( string.toUtf8() );
    readFields( object, document.object(), objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString ObjectJsonCapability::writeObjectToString( const ObjectHandle* object )
{
    QJsonObject jsonObject;
    jsonObject["classKeyword"] = object->capability<ObjectIoCapability>()->classKeyword();
    writeFields( object, jsonObject );
    QJsonDocument document( jsonObject );
    return QString( document.toJson() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::copyByJsonSerialization( const ObjectHandle* object,
                                                                   ObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString string = writeObjectToString( object );

    ObjectHandle* objectCopy = readUnknownObjectFromString( string, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                                          const QString&    destinationClassKeyword,
                                                                          const QString&    sourceClassKeyword,
                                                                          ObjectFactory* objectFactory )
{
    auto ioCapability = object->capability<ObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString string = writeObjectToString( object );

    ObjectHandle* upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    QJsonDocument document = QJsonDocument::fromJson( string.toUtf8() );
    readFields( upgradedObject, document.object(), objectFactory, true );

    upgradedObject->capability<ObjectIoCapability>()->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectJsonCapability::readUnknownObjectFromString( const QString&    string,
                                                                       ObjectFactory* objectFactory,
                                                                       bool              isCopyOperation )
{
    QJsonDocument document   = QJsonDocument::fromJson( string.toUtf8() );
    QJsonObject   jsonObject = document.object();

    auto jsonValue = jsonObject["classKeyword"];
    CAF_ASSERT( jsonValue.isString() );
    QString classKeyword = jsonValue.toString();

    ObjectHandle* newObject = objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFields( newObject, jsonObject, objectFactory, isCopyOperation );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readFile( ObjectHandle* object, QIODevice* file )
{
    CAF_ASSERT( file );

    QByteArray fullFile = file->readAll();

    QJsonDocument document = QJsonDocument::fromJson( fullFile );
    readFields( object, document.object(), PdmDefaultObjectFactory::instance(), false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::writeFile( const ObjectHandle* object, QIODevice* file )
{
    QJsonDocument document;
    QJsonObject   docObject;

    writeFields( object, docObject );
    document.setObject( docObject );
    file->write( document.toJson() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::readFields( ObjectHandle*   object,
                                          const QJsonObject& jsonObject,
                                          ObjectFactory*  objectFactory,
                                          bool               isCopyOperation )
{
    auto classKeyword = jsonObject["classKeyword"];
    CAF_ASSERT( classKeyword.toString() == object->capability<ObjectIoCapability>()->classKeyword() );
    for ( auto it = jsonObject.begin(); it != jsonObject.end(); ++it )
    {
        QString fieldName  = it.key();
        auto    fieldValue = it.value();

        auto fieldHandle = object->findField( fieldName );
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
                // readFieldData assumes that the xmlStream points to first token of field content.
                // After reading, the xmlStream is supposed to point to the first token after the field content.
                // (typically an "endElement")
                ioFieldHandle->readFieldData( fieldValue, objectFactory );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectJsonCapability::writeFields( const ObjectHandle* object, QJsonObject& jsonObject )
{
    std::vector<FieldHandle*> fields;
    object->fields( fields );
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const FieldIoCapability* ioCapability = fields[it]->capability<FieldIoCapability>();
        if ( ioCapability && ioCapability->isIOWritable() )
        {
            QString keyword = ioCapability->fieldHandle()->keyword();
            CAF_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

            QJsonValue value;
            ioCapability->writeFieldData( value );
            jsonObject[keyword] = value;
        }
    }
}
