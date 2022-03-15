
#include "cafAssert.h"
#include "cafLogger.h"
#include "cafObjectFactory.h"
#include "cafObjectIoCapability.h"
#include "cafSerializer.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

#include <iostream>

namespace caffa
{
//==================================================================================================
/// XML Implementation for FieldIoCap<> methods
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer )
{
    this->assertValid();
    if ( serializer.serializeDataValues() )
    {
        CAFFA_TRACE( "Setting value from json to: " << jsonElement.dump() );
        if ( jsonElement.is_object() )
        {
            if ( jsonElement.contains( "value" ) )
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
    if ( serializer.serializeSchema() )
    {
        if ( jsonElement.contains( "type" ) )
        {
            CAFFA_ASSERT( jsonElement["type"] == m_field->dataType() );
        }

        if ( m_field->valueValidator() )
        {
            m_field->valueValidator()->readFromJson( jsonElement, serializer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeToJson( nlohmann::json& jsonElement, const Serializer& serializer ) const
{
    this->assertValid();

    nlohmann::json jsonField = nlohmann::json::object();

    if ( serializer.serializeSchema() )
    {
        jsonField["type"] = m_field->dataType();
        if ( m_field->valueValidator() )
        {
            m_field->valueValidator()->writeToJson( jsonField, serializer );
        }
    }
    if ( serializer.serializeDataValues() )
    {
        CAFFA_TRACE( "Getting value from field: " << m_field );

        nlohmann::json jsonValue = m_field->value();

        jsonField["value"] = jsonValue;
    }

    jsonElement = jsonField;

    CAFFA_TRACE( "Writing field to json " << m_field->keyword() << "(" << m_field->dataType()
                                          << ") = " << jsonElement.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromJson( const nlohmann::json& jsonObject, const Serializer& serializer )
{
    CAFFA_TRACE( "Writing " << jsonObject.dump() << " to ChildField" );
    if ( jsonObject.is_null() ) return;
    CAFFA_ASSERT( jsonObject.is_object() );

    std::string className = jsonObject["Class"].get<std::string>();

    ObservingPointer<ObjectHandle> objPtr;

    // Create a new object
    {
        auto objectFactory = serializer.objectFactory();

        CAFFA_ASSERT( objectFactory );

        auto obj = objectFactory->create( className );
        if ( !obj )
        {
            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

            return;
        }
        else
        {
            objPtr = obj.get();

            auto ioObject = obj->template capability<caffa::ObjectIoCapability>();
            if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
            {
                CAFFA_ASSERT( false ); // Inconsistency in the factory. It creates objects of wrong type from the
                                       // ClassKeyword
                return;
            }
            m_field->setChildObject( std::move( obj ) );
        }
    }

    auto ioObject = objPtr->template capability<caffa::ObjectIoCapability>();
    if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
    {
        // Error: Field contains different class type than on file
        std::cout << "Warning: Unknown object type with class name: " << className.c_str()
                  << " found while reading the field : " << m_field->keyword().c_str() << std::endl;
        std::cout << "                     Expected class name: " << ioObject->classKeyword().c_str() << std::endl;

        return;
    }

    // Everything seems ok, so read the contents of the object:
    CAFFA_ASSERT( objPtr.notNull() );
    std::string jsonString = jsonObject.dump();
    serializer.readObjectFromString( objPtr.p(), jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeToJson( nlohmann::json& jsonValue, const Serializer& serializer ) const
{
    auto childObjects = m_field->childObjects();
    if ( childObjects.empty() ) return;

    auto object = childObjects.front();

    auto ioObject = object->template capability<caffa::ObjectIoCapability>();
    if ( ioObject )
    {
        std::string jsonString = serializer.writeObjectToString( object );

        nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
        CAFFA_ASSERT( jsonObject.is_object() );
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromJson( const nlohmann::json& jsonValue, const Serializer& serializer )
{
    m_field->clear();

    CAFFA_TRACE( "Writing " << jsonValue.dump() << " to ChildArrayField" );

    if ( !jsonValue.is_array() ) return;

    auto objectFactory = serializer.objectFactory();
    CAFFA_ASSERT( objectFactory );

    for ( const auto& jsonObject : jsonValue )
    {
        if ( !jsonObject.is_object() ) continue;

        std::string className = jsonObject["Class"].get<std::string>();

        std::unique_ptr<ObjectHandle> obj = objectFactory->create( className );

        if ( !obj )
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Warning: Unknown object type with class name: " << className
                      << " found while reading the field : " << m_field->keyword() << std::endl;

            continue;
        }

        auto ioObject = obj->template capability<caffa::ObjectIoCapability>();
        if ( !ioObject || !ioObject->matchesClassKeyword( className ) )
        {
            CAFFA_ASSERT( false ); // There is an inconsistency in the factory. It creates objects of type not matching
                                   // the ClassKeyword

            continue;
        }

        std::string jsonString = jsonObject.dump();
        serializer.readObjectFromString( obj.get(), jsonString );

        size_t currentSize = m_field->size();
        m_field->insertAt( currentSize, std::move( obj ) );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToJson( nlohmann::json& jsonValue, const Serializer& serializer ) const
{
    typename std::vector<ObservingPointer<DataType>>::iterator it;

    nlohmann::json jsonArray = nlohmann::json::array();

    for ( size_t i = 0; i < m_field->size(); ++i )
    {
        ObjectHandle* object = m_field->at( i );
        if ( !object ) continue;

        auto ioObject = object->capability<caffa::ObjectIoCapability>();
        if ( ioObject )
        {
            std::string    jsonString = serializer.writeObjectToString( object );
            nlohmann::json jsonObject = nlohmann::json::parse( jsonString );
            jsonArray.push_back( jsonObject );
        }
    }
    jsonValue = jsonArray;
}
} // End namespace caffa
