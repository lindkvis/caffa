
#include "cafAssert.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObjectFactory.h"

#include <nlohmann/json.hpp>

#include <iostream>

namespace caffa
{
//==================================================================================================
/// XML Implementation for FieldJsonCap<> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldJsonCap<FieldType>::readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer )
{
    this->assertValid();
    if ( serializer.writeTypesAndValidators() )
    {
        CAFFA_TRACE( "Setting value from json to: " << jsonElement.dump() );
        if ( jsonElement.is_object() )
        {
            if ( jsonElement.contains( "value" ) && !jsonElement["value"].is_null() )
            {
                typename FieldType::FieldDataType value = jsonElement["value"].get<typename FieldType::FieldDataType>();
                m_field->setValue( value );
            }
        }
        else // Support JSON objects with direct value instead of separate value entry
        {
            typename FieldType::FieldDataType value = jsonElement.get<typename FieldType::FieldDataType>();
            m_field->setValue( value );
        }
    }

    if ( jsonElement.contains( "type" ) )
    {
        if ( jsonElement["type"] != m_field->dataType() )
        {
            CAFFA_ERROR( "Data type of json '" << jsonElement["type"] << "' does not match field data type '"
                                               << m_field->dataType() << "'" );
            CAFFA_ASSERT( false );
        }
    }

    for ( auto validator : m_field->valueValidators() )
    {
        validator->readFromJson( jsonElement, serializer );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldJsonCap<FieldType>::writeToJson( nlohmann::json& jsonElement, const Serializer& serializer ) const
{
    this->assertValid();

    nlohmann::json jsonField = nlohmann::json::object();

    if ( serializer.serializeDataTypes() )
    {
        jsonField["type"] = m_field->dataType();
        for ( auto validator : m_field->valueValidators() )
        {
            validator->writeToJson( jsonField, serializer );
        }
    }
    if ( serializer.writeTypesAndValidators() )
    {
        CAFFA_TRACE( "Getting value from field: " << m_field );

        nlohmann::json jsonValue = m_field->value();

        if ( serializer.serializeDataTypes() )
        {
            jsonField["value"] = jsonValue;
        }
        else
        {
            jsonField = jsonValue;
        }
    }

    jsonElement = jsonField;

    CAFFA_TRACE( "Writing field to json " << m_field->keyword() << "(" << m_field->dataType() << ") = " );
    CAFFA_TRACE( jsonElement.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
const FieldHandle* FieldJsonCap<FieldType>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldJsonCap<FieldType>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<FieldType*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::shared_ptr<DataType>>>::readFromJson( const nlohmann::json& jsonElement,
                                                                   const Serializer&     serializer )
{
    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildField" );
    if ( jsonElement.is_null() ) return;
    CAFFA_ASSERT( jsonElement.is_object() );

    nlohmann::json jsonObject;
    if ( jsonElement.contains( "value" ) )
    {
        jsonObject = jsonElement["value"];
    }
    else
    {
        jsonObject = jsonElement;
    }

    std::string className = jsonObject["class"].get<std::string>();
    std::string uuid      = "";
    if ( jsonObject.contains( "uuid" ) )
    {
        uuid = jsonObject["uuid"].get<std::string>();
    }

    auto object = m_field->value();
    if ( object && !uuid.empty() && object->uuid() == uuid )
    {
        CAFFA_TRACE( "Had existing object! Overwriting values!" );
    }
    else
    {
        // Create a new object
        auto objectFactory = serializer.objectFactory();

        CAFFA_ASSERT( objectFactory );

        object = std::dynamic_pointer_cast<DataType>( objectFactory->create( className ) );
        if ( !object )
        {
            CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                                 << m_field->keyword() );
            return;
        }
        else
        {
            m_field->setValue( object );
        }
    }

    CAFFA_ASSERT( object );
    if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
    {
        // Error: Field contains different class type than in the JSON
        CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                             << m_field->keyword() );
        CAFFA_ERROR( "                     Expected class name: " << object->classKeyword() );

        return;
    }

    // Everything seems ok, so read the contents of the object:
    std::string jsonString = jsonObject.dump();
    serializer.readObjectFromString( object.get(), jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::shared_ptr<DataType>>>::writeToJson( nlohmann::json&   jsonValue,
                                                                  const Serializer& serializer ) const
{
    auto object = m_field->value();
    if ( !object ) return;

    std::string    jsonString = serializer.writeObjectToString( object.get() );
    nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
    CAFFA_ASSERT( jsonObject.is_object() );
    if ( serializer.serializeDataTypes() )
    {
        jsonValue["type"]  = "object";
        jsonValue["value"] = jsonObject;
    }
    else
    {
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldJsonCap<Field<std::shared_ptr<DataType>>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::shared_ptr<DataType>>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<Field<std::shared_ptr<DataType>>*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::vector<std::shared_ptr<DataType>>>>::readFromJson( const nlohmann::json& jsonElement,
                                                                                const Serializer&     serializer )
{
    m_field->clear();

    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildArrayField" );

    nlohmann::json jsonArray;
    if ( jsonElement.is_array() )
    {
        jsonArray = jsonElement;
    }
    else if ( jsonElement.is_object() && jsonElement.contains( "value" ) )
    {
        jsonArray = jsonElement["value"];
    }

    CAFFA_ASSERT( jsonArray.is_array() );

    auto objectFactory = serializer.objectFactory();
    CAFFA_ASSERT( objectFactory );

    for ( const auto& jsonObject : jsonArray )
    {
        if ( !jsonObject.is_object() ) continue;

        std::string className = jsonObject["class"].get<std::string>();

        ObjectHandle::Ptr object = objectFactory->create( className );

        if ( !object )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

            continue;
        }

        if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
        {
            CAFFA_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                   // the ClassKeyword

            continue;
        }

        std::string jsonString = jsonObject.dump();
        serializer.readObjectFromString( object.get(), jsonString );

        size_t currentSize = m_field->size();
        m_field->insertAt( currentSize, object );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::vector<std::shared_ptr<DataType>>>>::writeToJson( nlohmann::json&   jsonValue,
                                                                               const Serializer& serializer ) const
{
    nlohmann::json jsonArray = nlohmann::json::array();

    for ( size_t i = 0; i < m_field->size(); ++i )
    {
        ObjectHandle::Ptr object = m_field->at( i );
        if ( !object ) continue;

        std::string    jsonString = serializer.writeObjectToString( object.get() );
        nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
        jsonArray.push_back( jsonObject );
    }

    if ( serializer.serializeDataTypes() )
    {
        jsonValue["type"]  = "object[]";
        jsonValue["value"] = jsonArray;
    }
    else
    {
        jsonValue = jsonArray;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldJsonCap<Field<std::vector<std::shared_ptr<DataType>>>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<Field<std::vector<std::shared_ptr<DataType>>>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<FieldType*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildField<DataType*>>::readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer )
{
    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildField" );
    if ( jsonElement.is_null() ) return;
    CAFFA_ASSERT( jsonElement.is_object() );

    nlohmann::json jsonObject;
    if ( jsonElement.contains( "value" ) )
    {
        jsonObject = jsonElement["value"];
    }
    else
    {
        jsonObject = jsonElement;
    }

    std::string className = jsonObject["class"].get<std::string>();
    std::string uuid      = "";
    if ( jsonObject.contains( "uuid" ) )
    {
        uuid = jsonObject["uuid"].get<std::string>();
    }

    auto object = m_field->object();
    if ( object && !uuid.empty() && object->uuid() == uuid )
    {
        CAFFA_TRACE( "Had existing object! Overwriting values!" );
    }
    else
    {
        // Create a new object
        auto objectFactory = serializer.objectFactory();

        CAFFA_ASSERT( objectFactory );

        object = std::dynamic_pointer_cast<DataType>( objectFactory->create( className ) );
        if ( !object )
        {
            CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                                 << m_field->keyword() );
            return;
        }
        else
        {
            m_field->setObject( object );
        }
    }
    if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
    {
        // Error: Field contains different class type than in the JSON
        CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                             << m_field->keyword() );
        CAFFA_ERROR( "                     Expected class name: " << object->classKeyword() );

        return;
    }

    // Everything seems ok, so read the contents of the object:
    std::string jsonString = jsonObject.dump();
    serializer.readObjectFromString( object.get(), jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildField<DataType*>>::writeToJson( nlohmann::json& jsonValue, const Serializer& serializer ) const
{
    auto object = m_field->object();
    if ( !object ) return;

    std::string    jsonString = serializer.writeObjectToString( object.get() );
    nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
    CAFFA_ASSERT( jsonObject.is_object() );
    if ( serializer.serializeDataTypes() )
    {
        jsonValue["type"]  = "object";
        jsonValue["value"] = jsonObject;
    }
    else
    {
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldJsonCap<ChildField<DataType*>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildField<DataType*>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<ChildField<DataType*>*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildArrayField<DataType*>>::readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer )
{
    m_field->clear();

    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildArrayField" );

    nlohmann::json jsonArray;
    if ( jsonElement.is_array() )
    {
        jsonArray = jsonElement;
    }
    else if ( jsonElement.is_object() && jsonElement.contains( "value" ) )
    {
        jsonArray = jsonElement["value"];
    }

    CAFFA_ASSERT( jsonArray.is_array() );

    auto objectFactory = serializer.objectFactory();
    CAFFA_ASSERT( objectFactory );

    for ( const auto& jsonObject : jsonArray )
    {
        if ( !jsonObject.is_object() ) continue;

        std::string className = jsonObject["class"].get<std::string>();

        ObjectHandle::Ptr object = objectFactory->create( className );

        if ( !object )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

            continue;
        }

        if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
        {
            CAFFA_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                   // the ClassKeyword

            continue;
        }

        std::string jsonString = jsonObject.dump();
        serializer.readObjectFromString( object.get(), jsonString );

        size_t currentSize = m_field->size();
        m_field->insertAt( currentSize, object );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildArrayField<DataType*>>::writeToJson( nlohmann::json& jsonValue, const Serializer& serializer ) const
{
    nlohmann::json jsonArray = nlohmann::json::array();

    for ( size_t i = 0; i < m_field->size(); ++i )
    {
        ObjectHandle::Ptr object = m_field->at( i );
        if ( !object ) continue;

        std::string    jsonString = serializer.writeObjectToString( object.get() );
        nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
        jsonArray.push_back( jsonObject );
    }

    if ( serializer.serializeDataTypes() )
    {
        jsonValue["type"]  = "object[]";
        jsonValue["value"] = jsonArray;
    }
    else
    {
        jsonValue = jsonArray;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldJsonCap<ChildArrayField<DataType*>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldJsonCap<ChildArrayField<DataType*>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<ChildArrayField<DataType*>*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

} // End namespace caffa
