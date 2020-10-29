
#include "cafAssert.h"
#include "cafInternalFieldIoHelper.h"
#include "cafObjectFactory.h"
#include "cafObjectIoCapability.h"
#include "cafObjectJsonCapability.h"
#include "cafObjectXmlCapability.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include <iostream>

namespace caf
{
//==================================================================================================
/// XML Implementation for FieldIoCap<> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool FieldIoCap<FieldType>::isVectorField() const
{
    return is_vector<FieldType>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    FieldReader<typename FieldType::FieldDataType>::readFieldData( value, xmlStream, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();
    FieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    FieldReader<typename FieldType::FieldDataType>::readFieldData( value, jsonValue, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();
    FieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), jsonValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool FieldIoCap<FieldType>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for PtrField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void FieldIoCap<PtrField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* )
{
    this->assertValid();

    FieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString dataString = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    FieldIOHelper::skipCharactersAndComments( xmlStream );

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
void FieldIoCap<PtrField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
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
void FieldIoCap<PtrField<DataType*>>::readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory )
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
void FieldIoCap<PtrField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
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
bool FieldIoCap<PtrField<DataType*>>::resolveReferences()
{
    if ( m_isResolved ) return true;
    if ( m_referenceString.isEmpty() ) return true;

    ObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), m_referenceString );
    m_field->setRawPtr( objHandle );
    m_isResolved = true;

    return objHandle != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
QString FieldIoCap<PtrField<DataType*>>::referenceString() const
{
    return m_referenceString;
}

//==================================================================================================
/// XML Implementation for PtrArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void FieldIoCap<PtrArrayField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* )
{
    this->assertValid();

    FieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString dataString = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    FieldIOHelper::skipCharactersAndComments( xmlStream );

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
void FieldIoCap<PtrArrayField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
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
void FieldIoCap<PtrArrayField<DataType*>>::readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory )
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
void FieldIoCap<PtrArrayField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
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
bool FieldIoCap<PtrArrayField<DataType*>>::resolveReferences()
{
    if ( m_isResolved ) return true;
    if ( m_referenceString.isEmpty() ) return true;
    m_field->clear();

    bool        foundValidObjectFromString = true;
    QStringList tokens                     = m_referenceString.split( '|' );
    for ( int i = 0; i < tokens.size(); ++i )
    {
        ObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), tokens[i] );
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
QString FieldIoCap<PtrArrayField<DataType*>>::referenceString() const
{
    return m_referenceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<PtrArrayField<DataType*>>::isVectorField() const
{
    return true;
}

//==================================================================================================
/// XML Implementation for ChildField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory )
{
    FieldIOHelper::skipCharactersAndComments( xmlStream );
    if ( !xmlStream.isStartElement() )
    {
        return; // This happens when the field is "shortcut" empty (written like: <ElementName/>)
    }

    QString          className = xmlStream.name().toString();
    ObjectHandle* obj       = nullptr;

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
            auto ioObject = obj->template capability<caf::ObjectIoCapability>();
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

    auto ioObject = obj->template capability<caf::ObjectIoCapability>();
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

    ObjectXmlCapability::readFields( obj, xmlStream, objectFactory, false );

    // Make stream point to endElement of this field

    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    FieldIOHelper::skipCharactersAndComments( xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->template capability<caf::ObjectIoCapability>();
    if ( ioObject )
    {
        QString className = ioObject->classKeyword();

        xmlStream.writeStartElement( "", className );
        ObjectXmlCapability::writeFields( object, xmlStream );
        xmlStream.writeEndElement();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory )
{
    QJsonObject jsonObject = jsonValue.toObject();

    QString className = jsonObject["classKeyword"].toString();

    ObjectHandle* obj = nullptr;

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
            auto ioObject = obj->template capability<caf::ObjectIoCapability>();
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

    auto ioObject = obj->template capability<caf::ObjectIoCapability>();
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
    ObjectJsonCapability::readFields( obj, jsonObject, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->template capability<caf::ObjectIoCapability>();
    if ( ioObject )
    {
        QString className = ioObject->classKeyword();

        QJsonObject jsonObject;
        jsonObject["classKeyword"] = className;
        ObjectJsonCapability::writeFields( object, jsonObject );
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<ChildField<DataType*>>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for ChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->template capability<caf::ObjectIoCapability>();
        if ( ioObject )
        {
            QString className = ioObject->classKeyword();

            xmlStream.writeStartElement( "", className );
            ObjectXmlCapability::writeFields( it->rawPtr(), xmlStream );
            xmlStream.writeEndElement();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFieldData( QXmlStreamReader& xmlStream,
                                                                  ObjectFactory* objectFactory )
{
    m_field->deleteAllChildObjects();
    FieldIOHelper::skipCharactersAndComments( xmlStream );
    while ( xmlStream.isStartElement() )
    {
        QString className = xmlStream.name().toString();

        CAF_ASSERT( objectFactory );
        ObjectHandle* obj = objectFactory->create( className );

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
            FieldIOHelper::skipCharactersAndComments( xmlStream );

            continue;
        }

        auto ioObject = obj->template capability<caf::ObjectIoCapability>();
        if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
        {
            CAF_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                 // the ClassKeyword

            // Skip to EndElement of the object
            xmlStream.skipCurrentElement();

            // Jump off the end element, and head for next start element (or the final EndElement of the field)
            QXmlStreamReader::TokenType type = xmlStream.readNext();
            Q_UNUSED( type );
            FieldIOHelper::skipCharactersAndComments( xmlStream );

            continue;
        }

        ObjectXmlCapability::readFields( obj, xmlStream, objectFactory, false );

        m_field->m_pointers.push_back( PdmPointer<DataType>() );
        m_field->m_pointers.back().setRawPtr( obj );
        obj->setAsParentField( m_field );

        // Jump off the end element, and head for next start element (or the final EndElement of the field)
        // Qt reports a character token between EndElements and StartElements so skip it

        QXmlStreamReader::TokenType type = xmlStream.readNext();
        Q_UNUSED( type );
        FieldIOHelper::skipCharactersAndComments( xmlStream );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFieldData( const QJsonValue& jsonValue,
                                                                  ObjectFactory* objectFactory )
{
    m_field->deleteAllChildObjects();

    QJsonArray array = jsonValue.toArray();

    for ( auto it = array.begin(); it != array.end(); ++it )
    {
        auto        value      = *it;
        QJsonObject jsonObject = value.toObject();
        QString     className  = jsonObject["classKeyword"].toString();

        CAF_ASSERT( objectFactory );
        ObjectHandle* obj = objectFactory->create( className );

        if ( obj == nullptr )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className.toLatin1().data()
                      << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            continue;
        }

        auto ioObject = obj->template capability<caf::ObjectIoCapability>();
        if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
        {
            CAF_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                 // the ClassKeyword

            continue;
        }

        ObjectJsonCapability::readFields( obj, jsonObject, objectFactory, false );

        m_field->m_pointers.push_back( PdmPointer<DataType>() );
        m_field->m_pointers.back().setRawPtr( obj );
        obj->setAsParentField( m_field );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeFieldData( QJsonValue& jsonValue ) const
{
    typename std::vector<PdmPointer<DataType>>::iterator it;

    QJsonArray jsonArray;
    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->template capability<caf::ObjectIoCapability>();
        if ( ioObject )
        {
            QString     className = ioObject->classKeyword();
            QJsonObject jsonObject;
            jsonObject["classKeyword"] = className;
            ObjectJsonCapability::writeFields( it->rawPtr(), jsonObject );
            jsonArray.push_back( jsonObject );
        }
    }
    jsonValue = jsonArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<ChildArrayField<DataType*>>::resolveReferences()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<ChildArrayField<DataType*>>::isVectorField() const
{
    return true;
}

//==================================================================================================
/// XML Implementation for FieldIoCap<std::vector<DataType>> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<Field<std::vector<DataType>>>::isVectorField() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<Field<std::vector<DataType>>>::readFieldData( QXmlStreamReader& xmlStream,
                                                                    ObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    FieldReader<typename FieldType::FieldDataType>::readFieldData( value, xmlStream, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<Field<std::vector<DataType>>>::writeFieldData( QXmlStreamWriter& xmlStream ) const
{
    this->assertValid();
    FieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<Field<std::vector<DataType>>>::readFieldData( const QJsonValue& jsonValue,
                                                                    ObjectFactory* objectFactory )
{
    this->assertValid();
    typename FieldType::FieldDataType value;
    FieldReader<typename FieldType::FieldDataType>::readFieldData( value, jsonValue, objectFactory );
    m_field->setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<Field<std::vector<DataType>>>::writeFieldData( QJsonValue& jsonValue ) const
{
    this->assertValid();
    FieldWriter<typename FieldType::FieldDataType>::writeFieldData( m_field->value(), jsonValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
bool FieldIoCap<Field<std::vector<DataType>>>::resolveReferences()
{
    return true;
}

} // End namespace caf
