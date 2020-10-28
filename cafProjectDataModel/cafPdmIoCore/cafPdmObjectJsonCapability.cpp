#include "cafPdmObjectJsonCapability.h"

#include "cafAssert.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectIoCapability.h"

#include "cafPdmFieldHandle.h"

#include <QByteArray>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectJsonCapability::readObjectFromString( PdmObjectHandle*  object,
                                                    const QString&    string,
                                                    PdmObjectFactory* objectFactory )
{
    QJsonDocument document = QJsonDocument::fromJson( string.toUtf8() );
    readFields( object, document.object(), objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectJsonCapability::writeObjectToString( const PdmObjectHandle* object )
{
    QJsonObject jsonObject;
    jsonObject["classKeyword"] = object->capability<PdmObjectIoCapability>()->classKeyword();
    writeFields( object, jsonObject );
    QJsonDocument document( jsonObject );
    return QString( document.toJson() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmObjectJsonCapability::copyByJsonSerialization( const PdmObjectHandle* object,
                                                                   PdmObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<PdmObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString string = writeObjectToString( object );

    PdmObjectHandle* objectCopy = readUnknownObjectFromString( string, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<PdmObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmObjectJsonCapability::copyAndCastByJsonSerialization( const PdmObjectHandle* object,
                                                                          const QString&    destinationClassKeyword,
                                                                          const QString&    sourceClassKeyword,
                                                                          PdmObjectFactory* objectFactory )
{
    auto ioCapability = object->capability<PdmObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString string = writeObjectToString( object );

    PdmObjectHandle* upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    QJsonDocument document = QJsonDocument::fromJson( string.toUtf8() );
    readFields( upgradedObject, document.object(), objectFactory, true );

    upgradedObject->capability<PdmObjectIoCapability>()->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmObjectJsonCapability::readUnknownObjectFromString( const QString&    string,
                                                                       PdmObjectFactory* objectFactory,
                                                                       bool              isCopyOperation )
{
    QJsonDocument document   = QJsonDocument::fromJson( string.toUtf8() );
    QJsonObject   jsonObject = document.object();

    auto jsonValue = jsonObject["classKeyword"];
    CAF_ASSERT( jsonValue.isString() );
    QString classKeyword = jsonValue.toString();

    PdmObjectHandle* newObject = objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFields( newObject, jsonObject, objectFactory, isCopyOperation );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectJsonCapability::readFile( PdmObjectHandle* object, QIODevice* file )
{
    CAF_ASSERT( file );

    QByteArray fullFile = file->readAll();

    QJsonDocument document = QJsonDocument::fromJson( fullFile );
    readFields( object, document.object(), PdmDefaultObjectFactory::instance(), false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectJsonCapability::writeFile( const PdmObjectHandle* object, QIODevice* file )
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
void PdmObjectJsonCapability::readFields( PdmObjectHandle*   object,
                                          const QJsonObject& jsonObject,
                                          PdmObjectFactory*  objectFactory,
                                          bool               isCopyOperation )
{
    auto classKeyword = jsonObject["classKeyword"];
    CAF_ASSERT( classKeyword.toString() == object->capability<PdmObjectIoCapability>()->classKeyword() );
    for ( auto it = jsonObject.begin(); it != jsonObject.end(); ++it )
    {
        QString fieldName  = it.key();
        auto    fieldValue = it.value();

        auto fieldHandle = object->findField( fieldName );
        if ( fieldHandle && fieldHandle->capability<PdmFieldIoCapability>() )
        {
            auto ioFieldHandle = fieldHandle->capability<PdmFieldIoCapability>();
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
void PdmObjectJsonCapability::writeFields( const PdmObjectHandle* object, QJsonObject& jsonObject )
{
    std::vector<PdmFieldHandle*> fields;
    object->fields( fields );
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const PdmFieldIoCapability* ioCapability = fields[it]->capability<PdmFieldIoCapability>();
        if ( ioCapability && ioCapability->isIOWritable() )
        {
            QString keyword = ioCapability->fieldHandle()->keyword();
            CAF_ASSERT( PdmObjectIoCapability::isValidElementName( keyword ) );

            QJsonValue value;
            ioCapability->writeFieldData( value );
            jsonObject[keyword] = value;
        }
    }
}
