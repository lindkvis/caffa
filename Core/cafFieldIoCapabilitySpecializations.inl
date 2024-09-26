
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
void FieldIoCap<FieldType>::readFromJson( const json::value& jsonElement, const JsonSerializer& serializer )
{
    this->assertValid();

    if ( jsonElement.is_null() ) return;

    if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL )
    {
        CAFFA_TRACE( "Setting value from json to: " << json::dump( jsonElement ) );
        if ( jsonElement.is_null() )
        {
            if constexpr ( std::is_floating_point_v<typename FieldType::FieldDataType> )
            {
                typedOwner()->setValue( std::numeric_limits<typename FieldType::FieldDataType>::quiet_NaN() );
            }
        }
        else if ( const auto* jsonObject = jsonElement.if_object(); jsonObject )
        {
            if ( const auto* jsonValue = jsonObject->if_contains( "value" ); jsonValue && !jsonValue->is_null() )
            {
                typename FieldType::FieldDataType value = json::from_json<typename FieldType::FieldDataType>( *jsonValue );
                typedOwner()->setValue( value );
            }
            else
            {
                throw std::runtime_error( "Invalid CAFFA JSON" );
            }
        }
        else // Support JSON objects with direct value instead of separate value entry
        {
            typename FieldType::FieldDataType value = json::from_json<typename FieldType::FieldDataType>( jsonElement );
            typedOwner()->setValue( value );
        }
    }

    for ( auto validator : typedOwner()->valueValidators() )
    {
        validator->readFromString( json::dump( jsonElement ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeToJson( json::value& jsonElement, const JsonSerializer& serializer ) const
{
    this->assertValid();

    if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL )
    {
        jsonElement = json::to_json( typedOwner()->value() );
    }
    else if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        json::object jsonSchema = JsonDataType<typename FieldType::FieldDataType>::jsonType();
        if ( !typedOwner()->isReadable() && typedOwner()->isWritable() )
        {
            jsonSchema["writeOnly"] = true;
        }
        else if ( typedOwner()->isReadable() && !typedOwner()->isWritable() )
        {
            jsonSchema["readOnly"] = true;
        }

        if ( !typedOwner()->documentation().empty() )
        {
            jsonSchema["description"] = typedOwner()->documentation();
        }

        for ( auto validator : typedOwner()->valueValidators() )
        {
            auto validatorString = validator->writeToString();
            auto validatorJson   = json::parse( validatorString );
            if ( const auto* validatorObject = validatorJson.if_object(); validatorObject )
            {
                for ( const auto& [key, entry] : *validatorObject )
                {
                    jsonSchema[key] = entry;
                }
            }
        }
        jsonElement = jsonSchema;
    }

    CAFFA_TRACE( "Writing field to json " << typedOwner()->keyword() << "(" << typedOwner()->dataType() << ") = " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
json::object FieldIoCap<FieldType>::jsonType() const
{
    return JsonDataType<typename FieldType::FieldDataType>::jsonType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromJson( const json::value& jsonElement, const JsonSerializer& serializer )
{
    CAFFA_TRACE( "Writing " << json::dump( jsonElement ) << " to ChildField" );
    if ( jsonElement.is_null() )
    {
        typedOwner()->setChildObject( nullptr );
        return;
    }

    CAFFA_ASSERT( jsonElement.is_object() );
    const auto& jsonValue = jsonElement.get_object();

    json::value jsonContent;
    if ( auto it = jsonValue.find( "value" ); it != jsonValue.end() )
    {
        jsonContent = it->value();
    }
    else
    {
        jsonContent = jsonValue;
    }

    if ( jsonContent.is_null() )
    {
        typedOwner()->setChildObject( nullptr );
        return;
    }

    if ( !jsonContent.is_object() )
    {
        throw std::runtime_error( "JSON for child field value is not a valid JSON object" );
    }

    const auto& jsonObject = jsonContent.get_object();

    auto classNameElement = jsonObject.find( "keyword" );
    if ( classNameElement == jsonObject.end() )
    {
        classNameElement = jsonObject.find( "class" );
    }

    if ( classNameElement == jsonObject.end() )
    {
        CAFFA_ERROR( "JSON does not contain class keyword: " << json::dump( jsonContent ) );
    }

    const auto className = json::from_json<std::string>( classNameElement->value() );

    std::string uuid;
    if ( auto it = jsonObject.find( "uuid" ); it != jsonObject.end() )
    {
        uuid = json::from_json<std::string>( it->value() );
    }

    auto object = typedOwner()->object();
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
                                                                 << typedOwner()->keyword() );
            return;
        }
        typedOwner()->setObject( object );
    }

    if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
    {
        // Error: Field contains different class type than in the JSON
        CAFFA_ERROR( "Unknown object type with class name: " << className << " found while reading the field : "
                                                             << typedOwner()->keyword() );
        CAFFA_ERROR( "                     Expected class name: " << object->classKeyword() );

        return;
    }

    // Everything seems ok, so read the contents of the object:
    serializer.readObjectFromJson( object.get(), jsonObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeToJson( json::value& jsonElement, const JsonSerializer& serializer ) const
{
    if ( auto object = typedOwner()->object(); object )
    {
        json::object jsonObject;
        serializer.writeObjectToJson( object.get(), jsonObject );
        jsonElement = jsonObject;
    }

    if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        auto jsonObject = JsonDataType<DataType>::jsonType();
        if ( !typedOwner()->isReadable() && typedOwner()->isWritable() )
        {
            jsonObject["writeOnly"] = true;
        }
        else if ( typedOwner()->isReadable() && !typedOwner()->isWritable() )
        {
            jsonObject["readOnly"] = true;
        }
        if ( !typedOwner()->documentation().empty() )
        {
            jsonObject["description"] = typedOwner()->documentation();
        }
        jsonElement = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
json::object FieldIoCap<ChildField<DataType*>>::jsonType() const
{
    return JsonDataType<ChildField<DataType*>>::jsonType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromJson( const json::value& jsonElement, const JsonSerializer& serializer )
{
    typedOwner()->clear();

    CAFFA_TRACE( "Writing " << json::dump( jsonElement ) << " to ChildArrayField " << typedOwner()->keyword() );

    json::array jsonArray;
    if ( jsonElement.is_array() )
    {
        jsonArray = jsonElement.get_array();
    }
    else if ( const auto* jsonObject = jsonElement.if_object(); jsonObject )
    {
        if ( const auto it = jsonObject->find( "value" ); it != jsonObject->end() && it->value().is_array() )
        {
            jsonArray = it->value().get_array();
        }
    }

    auto objectFactory = serializer.objectFactory();

    if ( !objectFactory )
    {
        CAFFA_ASSERT( false && "No object factory!" );
        return;
    }

    for ( const auto& jsonEntry : jsonArray )
    {
        const auto* jsonObject = jsonEntry.if_object();
        if ( !jsonObject ) continue;

        auto classNameElement = jsonObject->find( "keyword" );
        if ( classNameElement == jsonObject->end() )
        {
            classNameElement = jsonObject->find( "class" );
        }
        CAFFA_ASSERT( classNameElement != jsonObject->end() );

        const auto className = json::from_json<std::string>( classNameElement->value() );

        std::shared_ptr<ObjectHandle> object = objectFactory->create( className );

        if ( !object )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            CAFFA_ERROR( "Warning: Unknown object type with class name: "
                         << className << " found while reading the field : " << typedOwner()->keyword() );

            continue;
        }

        if ( !ObjectHandle::matchesClassKeyword( className, object->classInheritanceStack() ) )
        {
            CAFFA_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not
                                   // matching the ClassKeyword

            continue;
        }

        serializer.readObjectFromJson( object.get(), *jsonObject );

        size_t currentSize = typedOwner()->size();
        CAFFA_TRACE( "Inserting new object into " << typedOwner()->keyword() << " at position " << currentSize );

        typedOwner()->insertAt( currentSize, object );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToJson( json::value& jsonElement, const JsonSerializer& serializer ) const
{
    if ( serializer.serializationType() == JsonSerializer::SerializationType::SCHEMA )
    {
        auto jsonObject = JsonDataType<DataType>::jsonType();
        if ( !typedOwner()->isReadable() && typedOwner()->isWritable() )
        {
            jsonObject["writeOnly"] = true;
        }
        else if ( typedOwner()->isReadable() && !typedOwner()->isWritable() )
        {
            jsonObject["readOnly"] = true;
        }
        if ( !typedOwner()->documentation().empty() )
        {
            jsonObject["description"] = typedOwner()->documentation();
        }
        jsonElement = jsonObject;
    }
    else if ( serializer.serializationType() == JsonSerializer::SerializationType::DATA_FULL ||
              serializer.serializationType() == JsonSerializer::SerializationType::DATA_SKELETON )
    {
        json::array jsonArray;

        for ( size_t i = 0; i < typedOwner()->size(); ++i )
        {
            std::shared_ptr<ObjectHandle> object = typedOwner()->at( i );
            if ( !object ) continue;

            json::object jsonValue;
            serializer.writeObjectToJson( object.get(), jsonValue );
            jsonArray.push_back( jsonValue );
        }
        jsonElement = jsonArray;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
json::object FieldIoCap<ChildArrayField<DataType*>>::jsonType() const
{
    return JsonDataType<ChildArrayField<DataType*>>::jsonType();
}

} // End namespace caffa
