
#include "cafAssert.h"
#include "cafInternalIoFieldReaderWriter.h"
#include "cafLogger.h"
#include "cafObjectFactory.h"
#include "cafObjectIoCapability.h"
#include "cafObjectJsonCapability.h"
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
void FieldIoCap<FieldType>::readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues )
{
    this->assertValid();
    if ( copyDataValues )
    {
        CAFFA_TRACE( "Setting value from json to: " << jsonValue.dump() );

        typename FieldType::FieldDataType value = jsonValue.get<typename FieldType::FieldDataType>();
        m_field->setValue( value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const
{
    this->assertValid();
    if ( copyDataValues )
    {
        CAFFA_TRACE( "Getting value from field: " << m_field );

        jsonValue = m_field->value();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromJson( const nlohmann::json& jsonObject,
                                                      ObjectFactory*        objectFactory,
                                                      bool                  copyDataValues )
{
    CAFFA_TRACE( "Writing " << jsonObject.dump() << " to ChildField" );
    if ( jsonObject.is_null() ) return;
    CAFFA_ASSERT( jsonObject.is_object() );

    std::string className = jsonObject["classKeyword"].get<std::string>();

    Pointer<ObjectHandle> objPtr;

    // Create a new object
    {
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
    ObjectJsonCapability::readFieldsFromJson( objPtr.p(), jsonObject, objectFactory, copyDataValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const
{
    auto childObjects = m_field->childObjects();
    if ( childObjects.empty() ) return;

    auto object = childObjects.front();

    auto ioObject = object->template capability<caffa::ObjectIoCapability>();
    if ( ioObject )
    {
        std::string className = ioObject->classKeyword();

        nlohmann::json jsonObject  = nlohmann::json::object();
        jsonObject["classKeyword"] = className.c_str();
        ObjectJsonCapability::writeFieldsToJson( object, jsonObject, copyDataValues );
        CAFFA_ASSERT( jsonObject.is_object() );
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromJson( const nlohmann::json& jsonValue,
                                                           ObjectFactory*        objectFactory,
                                                           bool                  copyDataValues )
{
    m_field->clear();

    CAFFA_TRACE( "Writing " << jsonValue.dump() << " to ChildArrayField" );

    if ( !jsonValue.is_array() ) return;

    for ( const auto& jsonObject : jsonValue )
    {
        if ( !jsonObject.is_object() ) continue;

        std::string className = jsonObject["classKeyword"].get<std::string>();

        CAFFA_ASSERT( objectFactory );
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

        ObjectJsonCapability::readFieldsFromJson( obj.get(), jsonObject, objectFactory, copyDataValues );

        size_t currentSize = m_field->size();
        m_field->insertAt( currentSize, std::move( obj ) );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const
{
    typename std::vector<Pointer<DataType>>::iterator it;

    nlohmann::json jsonArray = nlohmann::json::array();

    for ( size_t i = 0; i < m_field->size(); ++i )
    {
        ObjectHandle* pointer = m_field->at( i );
        if ( !pointer ) continue;

        auto ioObject = pointer->capability<caffa::ObjectIoCapability>();
        if ( ioObject )
        {
            std::string    className   = ioObject->classKeyword();
            nlohmann::json jsonObject  = nlohmann::json::object();
            jsonObject["classKeyword"] = className;
            ObjectJsonCapability::writeFieldsToJson( pointer, jsonObject, copyDataValues );
            jsonArray.push_back( jsonObject );
        }
    }
    jsonValue = jsonArray;
}
} // End namespace caffa
