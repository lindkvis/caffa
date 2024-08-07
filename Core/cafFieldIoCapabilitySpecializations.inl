
#pragma once
#include "cafAssert.h"
#include "cafJsonDataType.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObjectFactory.h"

#include <string>

namespace caffa
{
//==================================================================================================
/// Implementation for FieldJsonCap<> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer )
{
    this->assertValid();

    if ( jsonElement.is_null() ) return;

    if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL )
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

    for ( auto validator : m_field->valueValidators() )
    {
        validator->readFromString( jsonElement.dump() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const
{
    this->assertValid();

    if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL )
    {
        jsonElement = m_field->value();
    }
    else if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        nlohmann::json jsonField = JsonDataType<typename FieldType::FieldDataType>::jsonType();
        if ( !m_field->isReadable() && m_field->isWritable() )
        {
            jsonField["writeOnly"] = true;
        }
        else if ( m_field->isReadable() && !m_field->isWritable() )
        {
            jsonField["readOnly"] = true;
        }

        if ( !m_field->documentation().empty() )
        {
            jsonField["description"] = m_field->documentation();
        }

        for ( auto validator : m_field->valueValidators() )
        {
            auto validatorString = validator->writeToString();
            auto validatorJson   = nlohmann::json::parse( validatorString );
            for ( auto [key, entry] : validatorJson.items() )
            {
                jsonField[key] = entry;
            }
        }
        jsonElement = jsonField;
    }

    CAFFA_TRACE( "Writing field to json " << m_field->keyword() << "(" << m_field->dataType() << ") = " );
    CAFFA_TRACE( jsonElement.dump() );
}

template <typename FieldType>
void FieldIoCap<FieldType>::readFromString( const std::string& string )
{
    std::istringstream ss( string );

    typename FieldType::FieldDataType value{};
    auto&                             istream = static_cast<std::istream&>( ss );

    istream >> std::boolalpha >> value;
    m_field->setValue( value );
}

template <typename FieldType>
void FieldIoCap<FieldType>::writeToString( std::string& string ) const
{
    std::ostringstream ss;
    ss << std::boolalpha << ( m_field->value() );
    string = ss.str();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
nlohmann::json FieldIoCap<FieldType>::jsonType() const
{
    return JsonDataType<typename FieldType::FieldDataType>::jsonType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
const FieldHandle* FieldIoCap<FieldType>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<FieldType*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer )
{
    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildField" );
    if ( jsonElement.is_null() )
    {
        m_field->setChildObject( nullptr );
    }

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
    if ( jsonObject.is_null() )
    {
        m_field->setChildObject( nullptr );
    }

    auto classNameElement = jsonObject.find( "keyword" );
    if ( classNameElement == jsonObject.end() )
    {
        classNameElement = jsonObject.find( "class" );
    }

    if ( classNameElement == jsonObject.end() )
    {
        CAFFA_ERROR( "JSON does not contain class keyword: " << jsonObject.dump() );
    }

    CAFFA_ASSERT( classNameElement != jsonObject.end() );

    std::string className = *classNameElement;

    std::string uuid;
    if ( jsonObject.contains( "uuid" ) )
    {
        uuid = jsonObject["uuid"].get<std::string>();
    }

    auto object = m_field->object();
    if ( object && !uuid.empty() && object->uuid() == uuid )
    {
        CAFFA_TRACE( "Had existing matching object! Overwriting field values!" );
    }
    else
    {
        // Create a new object
        auto objectFactory = serializer.objectFactory();

        if ( !objectFactory )
        {
            CAFFA_ASSERT( false && "No object factory!" );
            return;
        }

        object = std::dynamic_pointer_cast<DataType>( objectFactory->create( className ) );
        if ( !object )
        {
            CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                                 << m_field->keyword() );
            return;
        }
        m_field->setObject( object );
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
void FieldIoCap<ChildField<DataType*>>::writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const
{
    if ( auto object = m_field->object(); object )
    {
        std::string jsonString = serializer.writeObjectToString( object.get() );
        jsonElement            = nlohmann::json::parse( jsonString );
        CAFFA_ASSERT( jsonElement.is_object() );
    }

    if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        jsonElement = JsonDataType<DataType>::jsonType();
        if ( !m_field->isReadable() && m_field->isWritable() )
        {
            jsonElement["writeOnly"] = true;
        }
        else if ( m_field->isReadable() && !m_field->isWritable() )
        {
            jsonElement["readOnly"] = true;
        }
        if ( !m_field->documentation().empty() )
        {
            jsonElement["description"] = m_field->documentation();
        }
    }
}

template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromString( const std::string& string )
{
    auto json = nlohmann::json::parse( string );
    readFromJson( json, JsonSerializer() );
}

template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeToString( std::string& string ) const
{
    nlohmann::json json;
    writeToJson( json, JsonSerializer() );
    string = json.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
nlohmann::json FieldIoCap<ChildField<DataType*>>::jsonType() const
{
    return JsonDataType<ChildField<DataType*>>::jsonType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldIoCap<ChildField<DataType*>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<ChildField<DataType*>*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromJson( const nlohmann::json& jsonElement,
                                                           const JsonSerializer& serializer )
{
    m_field->clear();

    CAFFA_TRACE( "Writing " << jsonElement.dump() << " to ChildArrayField " << m_field->keyword() );

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

    if ( !objectFactory )
    {
        CAFFA_ASSERT( false && "No object factory!" );
        return;
    }

    for ( const auto& jsonObject : jsonArray )
    {
        if ( !jsonObject.is_object() ) continue;

        auto classNameElement = jsonObject.find( "keyword" );
        if ( classNameElement == jsonObject.end() )
        {
            classNameElement = jsonObject.find( "class" );
        }
        CAFFA_ASSERT( classNameElement != jsonObject.end() );

        std::string className = *classNameElement;

        std::shared_ptr<ObjectHandle> object = objectFactory->create( className );

        if ( !object )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            CAFFA_ERROR( "Warning: Unknown object type with class name: "
                         << className << " found while reading the field : " << m_field->keyword() );

            continue;
        }

        if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
        {
            CAFFA_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not
                                   // matching the ClassKeyword

            continue;
        }

        serializer.readObjectFromString( object.get(), jsonObject.dump() );

        size_t currentSize = m_field->size();
        CAFFA_TRACE( "Inserting new object into " << m_field->keyword() << " at position " << currentSize );

        m_field->insertAt( currentSize, object );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const
{
    if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        jsonElement = JsonDataType<std::vector<DataType>>::jsonType();
        if ( !m_field->isReadable() && m_field->isWritable() )
        {
            jsonElement["writeOnly"] = true;
        }
        else if ( m_field->isReadable() && !m_field->isWritable() )
        {
            jsonElement["readOnly"] = true;
        }
        if ( !m_field->documentation().empty() )
        {
            jsonElement["description"] = m_field->documentation();
        }
    }
    else if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL ||
              serializer.serializationType() == JsonSerializer::SerializationType::DATA_SKELETON )
    {
        nlohmann::json jsonArray = nlohmann::json::array();

        for ( size_t i = 0; i < m_field->size(); ++i )
        {
            std::shared_ptr<ObjectHandle> object = m_field->at( i );
            if ( !object ) continue;

            std::string    jsonString = serializer.writeObjectToString( object.get() );
            nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
            jsonArray.push_back( jsonObject );
        }
        jsonElement = jsonArray;
    }
}

template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromString( const std::string& string )
{
    auto json = nlohmann::json::parse( string );
    readFromJson( json, JsonSerializer() );
}

template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToString( std::string& string ) const
{
    nlohmann::json json;
    writeToJson( json, JsonSerializer() );
    string = json.dump();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
nlohmann::json FieldIoCap<ChildArrayField<DataType*>>::jsonType() const
{
    return JsonDataType<ChildArrayField<DataType*>>::jsonType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const FieldHandle* FieldIoCap<ChildArrayField<DataType*>>::owner() const
{
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::setOwner( FieldHandle* owner )
{
    auto field = dynamic_cast<ChildArrayField<DataType*>*>( owner );
    CAFFA_ASSERT( field );
    m_field = field;
}

} // End namespace caffa
