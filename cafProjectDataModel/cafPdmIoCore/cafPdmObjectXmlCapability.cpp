#include "cafPdmObjectXmlCapability.h"

#include "cafAssert.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectIoCapability.h"

#include "cafPdmFieldHandle.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <iostream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Reads all the fields into a PdmObject
/// Assumes xmlStream points to the start element token of the PdmObject for which to read fields.
/// ( and not first token of object content)
/// This makes attribute based field storage possible.
/// Leaves the xmlStream pointing to the EndElement of the PdmObject.
//--------------------------------------------------------------------------------------------------
void PdmObjectXmlCapability::readFields( PdmObjectHandle*  object,
                                         QXmlStreamReader& xmlStream,
                                         PdmObjectFactory* objectFactory,
                                         bool              isCopyOperation )
{
    bool                        isObjectFinished = false;
    QXmlStreamReader::TokenType type;
    while ( !isObjectFinished )
    {
        type = xmlStream.readNext();

        switch ( type )
        {
            case QXmlStreamReader::StartElement:
            {
                QString name = xmlStream.name().toString();

                PdmFieldHandle* fieldHandle = object->findField( name );
                if ( fieldHandle && fieldHandle->capability<PdmFieldIoCapability>() )
                {
                    PdmFieldIoCapability* ioFieldHandle = fieldHandle->capability<PdmFieldIoCapability>();
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
                        QXmlStreamReader::TokenType tt;
                        tt = xmlStream.readNext();
                        ioFieldHandle->readFieldData( xmlStream, objectFactory );
                    }
                    else
                    {
                        xmlStream.skipCurrentElement();
                    }
                }
                else
                {
                    // Debug text is commented out, as this code is relatively often reached. Consider a new logging
                    // concept to receive this information
                    //
                    // std::cout << "Line " << xmlStream.lineNumber() << ": Warning: Could not find a field with name "
                    // << name.toLatin1().data() << " in the current object : " << classKeyword().toLatin1().data() <<
                    // std::endl;

                    xmlStream.skipCurrentElement();
                }
                break;
            }
            break;
            case QXmlStreamReader::EndElement:
            {
                // End of object.
                QString name     = xmlStream.name().toString(); // For debugging
                isObjectFinished = true;
            }
            break;
            case QXmlStreamReader::EndDocument:
            {
                // End of object.
                isObjectFinished = true;
            }
            break;
            default:
            {
                // Just read on
                // Todo: Error handling
            }
            break;
        }
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectXmlCapability::writeFields( const PdmObjectHandle* object, QXmlStreamWriter& xmlStream )
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

            xmlStream.writeStartElement( "", keyword );
            ioCapability->writeFieldData( xmlStream );
            xmlStream.writeEndElement();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectXmlCapability::readObjectFromXmlString( PdmObjectHandle*  object,
                                                      const QString&    xmlString,
                                                      PdmObjectFactory* objectFactory )
{
    // A valid XML is used to store field data of the object. The format of the XML is like this

    /*
        <classKeyword>
            <fieldKeywordA>value</fieldKeywordA>
            <fieldKeywordB>value</fieldKeywordB>
        </classKeyword>
    */

    QXmlStreamReader inputStream( xmlString );

    QXmlStreamReader::TokenType tt;
    tt                   = inputStream.readNext(); // Start of document
    tt                   = inputStream.readNext();
    QString classKeyword = inputStream.name().toString();
    CAF_ASSERT( classKeyword == object->capability<PdmObjectIoCapability>()->classKeyword() );

    readFields( object, inputStream, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmObjectXmlCapability::readUnknownObjectFromXmlString( const QString&    xmlString,
                                                                         PdmObjectFactory* objectFactory,
                                                                         bool              isCopyOperation )
{
    QXmlStreamReader inputStream( xmlString );

    QXmlStreamReader::TokenType tt;
    tt                            = inputStream.readNext(); // Start of document
    tt                            = inputStream.readNext();
    QString          classKeyword = inputStream.name().toString();
    PdmObjectHandle* newObject    = objectFactory->create( classKeyword );

    if ( !newObject ) return nullptr;

    readFields( newObject, inputStream, objectFactory, isCopyOperation );

    return newObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectXmlCapability::readFile( PdmObjectHandle* object, QIODevice* file )
{
    QXmlStreamReader xmlStream( file );

    while ( !xmlStream.atEnd() )
    {
        xmlStream.readNext();
        if ( xmlStream.isStartElement() )
        {
            if ( !object->capability<PdmObjectIoCapability>()->matchesClassKeyword( xmlStream.name().toString() ) )
            {
                // Error: This is not a Pdm based xml document
                return;
            }
            readFields( object, xmlStream, PdmDefaultObjectFactory::instance(), false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectXmlCapability::writeFile( const PdmObjectHandle* object, QIODevice* file )
{
    QXmlStreamWriter xmlStream( file );
    xmlStream.setAutoFormatting( true );

    xmlStream.writeStartDocument();
    QString className = object->capability<PdmObjectIoCapability>()->classKeyword();

    xmlStream.writeStartElement( "", className );
    writeFields( object, xmlStream );
    xmlStream.writeEndElement();

    xmlStream.writeEndDocument();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmObjectXmlCapability::copyByXmlSerialization( const PdmObjectHandle* object,
                                                                 PdmObjectFactory*      objectFactory )
{
    auto ioCapability = object->capability<PdmObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString xmlString = writeObjectToXmlString( object );

    PdmObjectHandle* objectCopy = readUnknownObjectFromXmlString( xmlString, objectFactory, true );
    if ( !objectCopy ) return nullptr;

    objectCopy->capability<PdmObjectIoCapability>()->initAfterReadRecursively();

    return objectCopy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* PdmObjectXmlCapability::copyAndCastByXmlSerialization( const PdmObjectHandle* object,
                                                                             const QString&    destinationClassKeyword,
                                                                             const QString&    sourceClassKeyword,
                                                                             PdmObjectFactory* objectFactory )
{
    auto ioCapability = object->capability<PdmObjectIoCapability>();
    ioCapability->setupBeforeSaveRecursively();

    QString xmlString = writeObjectToXmlString( object );

    PdmObjectHandle* upgradedObject = objectFactory->create( destinationClassKeyword );
    if ( !upgradedObject ) return nullptr;

    QXmlStreamReader inputStream( xmlString );

    QXmlStreamReader::TokenType tt;
    tt                   = inputStream.readNext(); // Start of document
    tt                   = inputStream.readNext();
    QString classKeyword = inputStream.name().toString();
    CAF_ASSERT( classKeyword == sourceClassKeyword );

    readFields( upgradedObject, inputStream, objectFactory, true );
    upgradedObject->capability<PdmObjectIoCapability>()->initAfterReadRecursively();

    return upgradedObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectXmlCapability::writeObjectToXmlString( const PdmObjectHandle* object )
{
    QString          xmlString;
    QXmlStreamWriter outputStream( &xmlString );
    outputStream.setAutoFormatting( true );

    outputStream.writeStartElement( "", object->capability<PdmObjectIoCapability>()->classKeyword() );
    writeFields( object, outputStream );
    outputStream.writeEndElement();
    return xmlString;
}

} // end namespace caf
