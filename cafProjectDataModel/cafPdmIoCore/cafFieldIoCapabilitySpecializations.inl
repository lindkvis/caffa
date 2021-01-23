
#include "cafAssert.h"
#include "cafObjectFactory.h"
#include "cafObjectIoCapability.h"
#include "cafObjectJsonCapability.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

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
void FieldIoCap<FieldType>::readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
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
void FieldIoCap<FieldType>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<PtrField<DataType*>>::readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
{
    this->assertValid();

    std::string dataString = jsonValue.get<std::string>();

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
void FieldIoCap<PtrField<DataType*>>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
{
    this->assertValid();

    std::string dataString;

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
    if ( m_referenceString.empty() ) return true;

    ObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), m_referenceString );
    m_field->setRawPtr( objHandle );
    m_isResolved = true;

    return objHandle != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::string FieldIoCap<PtrField<DataType*>>::referenceString() const
{
    return m_referenceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<PtrArrayField<DataType*>>::readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
{
    this->assertValid();

    std::string dataString = jsonValue.get<std::string>();

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
void FieldIoCap<PtrArrayField<DataType*>>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
{
    this->assertValid();

    std::string dataString;
    size_t      pointerCount = m_field->m_pointers.size();
    for ( size_t i = 0; i < pointerCount; ++i )
    {
        dataString += PdmReferenceHelper::referenceFromFieldToObject( m_field, m_field->m_pointers[i].rawPtr() );
        if ( !dataString.empty() && i < pointerCount - 1 ) dataString += " | \n\t";
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
    if ( m_referenceString.empty() ) return true;
    m_field->clear();

    bool                   foundValidObjectFromString = true;
    std::list<std::string> tokens                     = caf::StringTools::split( m_referenceString, "|" );
    for ( auto token : tokens )
    {
        ObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference( this->fieldHandle(), token );
        if ( !token.empty() && !objHandle )
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
std::string FieldIoCap<PtrArrayField<DataType*>>::referenceString() const
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFieldData( const nlohmann::json& jsonObject, ObjectFactory* objectFactory )
{
    if ( jsonObject.is_null() ) return;

    CAF_ASSERT( jsonObject.is_object() );

    std::string className = jsonObject["classKeyword"].get<std::string>();

    ObjectHandle* obj = nullptr;

    // Create an object if needed
    if ( m_field->value() == nullptr )
    {
        CAF_ASSERT( objectFactory );

        uint64_t serverAddress = 0u;
        auto     it            = jsonObject.find( "serverAddress" );
        if ( it != jsonObject.end() ) serverAddress = it->get<uint64_t>();

        obj = objectFactory->create( className, serverAddress );

        if ( obj == nullptr )
        {
            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

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
        std::cout << "Warning: Unknown object type with class name: " << className.c_str()
                  << " found while reading the field : " << m_field->keyword().c_str() << std::endl;
        std::cout << "                     Expected class name: " << ioObject->classKeyword().c_str() << std::endl;

        return;
    }

    // Everything seems ok, so read the contents of the object:
    ObjectJsonCapability::readFields( obj, jsonObject, objectFactory, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->template capability<caf::ObjectIoCapability>();
    if ( ioObject )
    {
        std::string className = ioObject->classKeyword();

        nlohmann::json jsonObject  = nlohmann::json::object();
        jsonObject["classKeyword"] = className.c_str();
        if (writeServerAddress)
        {
            jsonObject["serverAddress"] = reinterpret_cast<uint64_t>( object );
        }
        ObjectJsonCapability::writeFields( object, jsonObject, writeServerAddress );
        CAF_ASSERT( jsonObject.is_object() );
        jsonValue = jsonObject;
        CAF_ASSERT( jsonValue.is_object() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
{
    m_field->deleteAllChildObjects();

    if ( !jsonValue.is_array() ) return;

    for ( const auto& jsonObject : jsonValue )
    {
        if ( !jsonObject.is_object() ) continue;

        std::string className = jsonObject["classKeyword"].get<std::string>();        
        
        uint64_t serverAddress = 0u;
        auto it = jsonObject.find( "serverAddress" );
        if ( it != jsonObject.end() ) serverAddress = it->get<uint64_t>();

        CAF_ASSERT( objectFactory );
        ObjectHandle* obj = objectFactory->create( className, serverAddress );

        if ( obj == nullptr )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

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
void FieldIoCap<ChildArrayField<DataType*>>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
{
    typename std::vector<PdmPointer<DataType>>::iterator it;

    nlohmann::json jsonArray = nlohmann::json::array();

    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->template capability<caf::ObjectIoCapability>();
        if ( ioObject )
        {
            std::string    className   = ioObject->classKeyword();
            nlohmann::json jsonObject  = nlohmann::json::object();
            jsonObject["classKeyword"] = className;
            if ( writeServerAddress )
            {
                jsonObject["serverAddress"] = reinterpret_cast<uint64_t>( it->rawPtr() );
            }
            ObjectJsonCapability::writeFields( it->rawPtr(), jsonObject, writeServerAddress );
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
void FieldIoCap<Field<std::vector<DataType>>>::readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
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
void FieldIoCap<Field<std::vector<DataType>>>::writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const
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
