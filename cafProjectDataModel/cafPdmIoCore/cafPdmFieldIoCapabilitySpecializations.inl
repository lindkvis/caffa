
#include "cafAssert.h"
#include "cafInternalPdmFieldIoHelper.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmObjectIoCapability.h"
#include "cafPdmObjectJsonCapability.h"
#include "cafPdmObjectXmlCapability.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include <iostream>

namespace caf
{
//==================================================================================================
/// XML Implementation for PdmFieldIoCap<> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool PdmFieldIoCap<FieldType>::isVectorField() const
{
    return is_vector<FieldType>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void PdmFieldIoCap<FieldType>::readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    PdmFieldReader<typename FieldType::FieldDataType>::readFieldData( value, xmlStream, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void PdmFieldIoCap<FieldType>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();
    PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void PdmFieldIoCap<FieldType>::readFieldData( const QJsonValue& jsonValue, PdmObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    PdmFieldReader<typename FieldType::FieldDataType>::readFieldData( value, jsonValue, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void PdmFieldIoCap<FieldType>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();
    PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), jsonValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool PdmFieldIoCap<FieldType>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmPtrField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void PdmFieldIoCap<PdmPtrField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* )
{
    this->assertValid();

    PdmFieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString dataString = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );

    // This resolving can NOT be done here.
    // It must be done when we know that the complete hierarchy is read and created,
    // The object pointed to is not always read and created at this point in time.
    // We rather need to do something like :
    // m_refenceString = dataString;
    // m_isResolved = false;
    // m_field->setRawPtr(nullptr);
    //
    // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

    m_isResolved      = false;
    m_referenceString = dataString;
    m_field->setRawPtr( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmPtrField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();

    QString dataString;

    dataString = PdmReferenceHelper::referenceFromFieldToObject( m_field, m_field->m_fieldValue.rawPtr() );

    xmlStream.writeCharacters( dataString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmPtrField<DataType*>>::readFieldData( const QJsonValue& jsonValue, PdmObjectFactory* objectFactory )
{
    this->assertValid();

    QString dataString = jsonValue.toString();

    // This resolving can NOT be done here.
    // It must be done when we know that the complete hierarchy is read and created,
    // The object pointed to is not always read and created at this point in time.
    // We rather need to do something like :
    // m_refenceString = dataString;
    // m_isResolved = false;
    // m_field->setRawPtr(nullptr);
    //
    // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

    m_isResolved      = false;
    m_referenceString = dataString;
    m_field->setRawPtr( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmPtrField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();

    QString dataString;

    dataString = PdmReferenceHelper::referenceFromFieldToObject( m_field, m_field->m_fieldValue.rawPtr() );
    jsonValue  = dataString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmPtrField<DataType*>>::resolveReferences()
{
    if ( m_isResolved ) return true;
    if ( m_referenceString.isEmpty() ) return true;

    PdmObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), m_referenceString );
    m_field->setRawPtr( objHandle );
    m_isResolved = true;

    return objHandle != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
QString PdmFieldIoCap<PdmPtrField<DataType*>>::referenceString() const
{
    return m_referenceString;
}

//==================================================================================================
/// XML Implementation for PdmPtrArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void PdmFieldIoCap<PdmPtrArrayField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* )
{
    this->assertValid();

    PdmFieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString dataString = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );

    // This resolving can NOT be done here.
    // It must be done when we know that the complete hierarchy is read and created,
    // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

    m_isResolved      = false;
    m_referenceString = dataString;
    m_field->clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void PdmFieldIoCap<PdmPtrArrayField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();

    QString dataString;
    size_t  pointerCount = m_field->m_pointers.size();
    for ( size_t i = 0; i < pointerCount; ++i )
    {
        dataString += PdmReferenceHelper::referenceFromFieldToObject( m_field, m_field->m_pointers[i].rawPtr() );
        if ( !dataString.isEmpty() && i < pointerCount - 1 ) dataString += " | \n\t";
    }
    xmlStream.writeCharacters( dataString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmPtrArrayField<DataType*>>::readFieldData( const QJsonValue& jsonValue, PdmObjectFactory* objectFactory )
{
    this->assertValid();

    QString dataString = jsonValue.toString();

    // This resolving can NOT be done here.
    // It must be done when we know that the complete hierarchy is read and created,
    // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

    m_isResolved      = false;
    m_referenceString = dataString;
    m_field->clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmPtrArrayField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();

    QString dataString;
    size_t  pointerCount = m_field->m_pointers.size();
    for ( size_t i = 0; i < pointerCount; ++i )
    {
        dataString += PdmReferenceHelper::referenceFromFieldToObject( m_field, m_field->m_pointers[i].rawPtr() );
        if ( !dataString.isEmpty() && i < pointerCount - 1 ) dataString += " | \n\t";
    }
    jsonValue = dataString;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmPtrArrayField<DataType*>>::resolveReferences()
{
    if ( m_isResolved ) return true;
    if ( m_referenceString.isEmpty() ) return true;
    m_field->clear();

    bool        foundValidObjectFromString = true;
    QStringList tokens                     = m_referenceString.split( '|' );
    for ( int i = 0; i < tokens.size(); ++i )
    {
        PdmObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), tokens[i] );
        if ( !tokens[i].isEmpty() && !objHandle )
        {
            foundValidObjectFromString = false;
        }

        m_field->m_pointers.push_back( nullptr );
        m_field->m_pointers.back().setRawPtr( objHandle );
    }

    m_isResolved = true;

    return foundValidObjectFromString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
QString PdmFieldIoCap<PdmPtrArrayField<DataType*>>::referenceString() const
{
    return m_referenceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmPtrArrayField<DataType*>>::isVectorField() const
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmChildField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void PdmFieldIoCap<PdmChildField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory )
{
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
    if ( !xmlStream.isStartElement() )
    {
        return; // This happens when the field is "shortcut" empty (written like: <ElementName/>)
    }

    QString          className = xmlStream.name().toString();
    PdmObjectHandle* obj       = nullptr;

    // Create an object if needed
    if ( m_field->value() == nullptr )
    {
        CAF_ASSERT( objectFactory );
        obj = objectFactory->create( className );

        if ( obj == nullptr )
        {
            std::cout << "Line " << xmlStream.lineNumber()
                      << ": Warning: Unknown object type with class name: " << className.toLatin1().data()
                      << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
            xmlStream.skipCurrentElement(); // Skip to the endelement of this field
            return;
        }
        else
        {
            auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
            if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
            {
                CAF_ASSERT( false ); // Inconsistency in the factory. It creates objects of wrong type from the
                                     // ClassKeyword

                xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
                xmlStream.skipCurrentElement(); // Skip to the endelement of this field

                return;
            }

            m_field->m_fieldValue.setRawPtr( obj );
            obj->setAsParentField( m_field );
        }
    }
    else
    {
        obj = m_field->m_fieldValue.rawPtr();
    }

    auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
    if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
    {
        // Error: Field contains different class type than on file
        std::cout << "Line " << xmlStream.lineNumber()
                  << ": Warning: Unknown object type with class name: " << className.toLatin1().data()
                  << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;
        std::cout << "                     Expected class name: " << ioObject->classKeyword().toLatin1().data()
                  << std::endl;

        xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
        xmlStream.skipCurrentElement(); // Skip to the endelement of this field

        return;
    }

    // Everything seems ok, so read the contents of the object:

    PdmObjectXmlCapability::readFields( obj, xmlStream, objectFactory, false );

    // Make stream point to endElement of this field

    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->capability<caf::PdmObjectIoCapability>();
    if ( ioObject )
    {
        QString className = ioObject->classKeyword();

        xmlStream.writeStartElement( "", className );
        PdmObjectXmlCapability::writeFields( object, xmlStream );
        xmlStream.writeEndElement();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildField<DataType*>>::readFieldData( const QJsonValue& jsonValue, PdmObjectFactory* objectFactory )
{
    QJsonObject jsonObject = jsonValue.toObject();

    QString className = jsonObject["classKeyword"].toString();

    PdmObjectHandle* obj = nullptr;

    // Create an object if needed
    if ( m_field->value() == nullptr )
    {
        CAF_ASSERT( objectFactory );
        obj = objectFactory->create( className );

        if ( obj == nullptr )
        {
            std::cout << "Warning: Unknown object type with class name: " << className.toLatin1().data()
                      << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            return;
        }
        else
        {
            auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
            if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
            {
                CAF_ASSERT( false ); // Inconsistency in the factory. It creates objects of wrong type from the
                                     // ClassKeyword
                return;
            }

            m_field->m_fieldValue.setRawPtr( obj );
            obj->setAsParentField( m_field );
        }
    }
    else
    {
        obj = m_field->m_fieldValue.rawPtr();
    }

    auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
    if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
    {
        // Error: Field contains different class type than on file
        std::cout << "Warning: Unknown object type with class name: " << className.toLatin1().data()
                  << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;
        std::cout << "                     Expected class name: " << ioObject->classKeyword().toLatin1().data()
                  << std::endl;

        return;
    }

    // Everything seems ok, so read the contents of the object:
    PdmObjectJsonCapability::readFields( obj, jsonObject, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->capability<caf::PdmObjectIoCapability>();
    if ( ioObject )
    {
        QString className = ioObject->classKeyword();

        QJsonObject jsonObject;
        jsonObject["classKeyword"] = className;
        PdmObjectJsonCapability::writeFields( object, jsonObject );
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmChildField<DataType*>>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildArrayField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->capability<caf::PdmObjectIoCapability>();
        if ( ioObject )
        {
            QString className = ioObject->classKeyword();

            xmlStream.writeStartElement( "", className );
            PdmObjectXmlCapability::writeFields( it->rawPtr(), xmlStream );
            xmlStream.writeEndElement();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildArrayField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream,
                                                                  PdmObjectFactory* objectFactory )
{
    m_field->deleteAllChildObjects();
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
    while ( xmlStream.isStartElement() )
    {
        QString className = xmlStream.name().toString();

        CAF_ASSERT( objectFactory );
        PdmObjectHandle* obj = objectFactory->create( className );

        if ( obj == nullptr )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Line " << xmlStream.lineNumber()
                      << ": Warning: Unknown object type with class name: " << className.toLatin1().data()
                      << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            // Skip to EndElement of the object
            xmlStream.skipCurrentElement();

            // Jump off the end element, and head for next start element (or the final EndElement of the field)
            QXmlStreamReader::TokenType type = xmlStream.readNext();
            Q_UNUSED( type );
            PdmFieldIOHelper::skipCharactersAndComments( xmlStream );

            continue;
        }

        auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
        if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
        {
            CAF_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                 // the ClassKeyword

            // Skip to EndElement of the object
            xmlStream.skipCurrentElement();

            // Jump off the end element, and head for next start element (or the final EndElement of the field)
            QXmlStreamReader::TokenType type = xmlStream.readNext();
            Q_UNUSED( type );
            PdmFieldIOHelper::skipCharactersAndComments( xmlStream );

            continue;
        }

        PdmObjectXmlCapability::readFields( obj, xmlStream, objectFactory, false );

        m_field->m_pointers.push_back( PdmPointer<DataType>() );
        m_field->m_pointers.back().setRawPtr( obj );
        obj->setAsParentField( m_field );

        // Jump off the end element, and head for next start element (or the final EndElement of the field)
        // Qt reports a character token between EndElements and StartElements so skip it

        QXmlStreamReader::TokenType type = xmlStream.readNext();
        Q_UNUSED( type );
        PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildArrayField<DataType*>>::readFieldData( const QJsonValue& jsonValue,
                                                                  PdmObjectFactory* objectFactory )
{
    m_field->deleteAllChildObjects();

    QJsonArray array = jsonValue.toArray();

    for ( auto it = array.begin(); it != array.end(); ++it )
    {
        auto        value      = *it;
        QJsonObject jsonObject = value.toObject();
        QString     className  = jsonObject["classKeyword"].toString();

        CAF_ASSERT( objectFactory );
        PdmObjectHandle* obj = objectFactory->create( className );

        if ( obj == nullptr )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className.toLatin1().data()
                      << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            continue;
        }

        auto ioObject = obj->capability<caf::PdmObjectIoCapability>();
        if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
        {
            CAF_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                 // the ClassKeyword

            continue;
        }

        PdmObjectJsonCapability::readFields( obj, jsonObject, objectFactory, false );

        m_field->m_pointers.push_back( PdmPointer<DataType>() );
        m_field->m_pointers.back().setRawPtr( obj );
        obj->setAsParentField( m_field );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmChildArrayField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    typename std::vector<PdmPointer<DataType>>::iterator it;

    QJsonArray jsonArray;
    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->capability<caf::PdmObjectIoCapability>();
        if ( ioObject )
        {
            QString     className = ioObject->classKeyword();
            QJsonObject jsonObject;
            jsonObject["classKeyword"] = className;
            PdmObjectJsonCapability::writeFields( it->rawPtr(), jsonObject );
            jsonArray.push_back( jsonObject );
        }
    }
    jsonValue = jsonArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmChildArrayField<DataType*>>::resolveReferences()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmChildArrayField<DataType*>>::isVectorField() const
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmFieldIoCap<std::vector<DataType>> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmField<std::vector<DataType>>>::isVectorField() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmField<std::vector<DataType>>>::readFieldData( QXmlStreamReader& xmlStream,
                                                                    PdmObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    PdmFieldReader<typename FieldType::FieldDataType>::readFieldData( value, xmlStream, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmField<std::vector<DataType>>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();
    PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmField<std::vector<DataType>>>::readFieldData( const QJsonValue& jsonValue,
                                                                    PdmObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    PdmFieldReader<typename FieldType::FieldDataType>::readFieldData( value, jsonValue, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldIoCap<PdmField<std::vector<DataType>>>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();
    PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), jsonValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool PdmFieldIoCap<PdmField<std::vector<DataType>>>::resolveReferences()
{
    return true;
}

} // End namespace caf
