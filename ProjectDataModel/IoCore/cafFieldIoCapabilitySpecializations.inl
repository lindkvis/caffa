
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
void FieldIoCap<FieldType>::writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues )
{
    this->assertValid();
    if ( copyDataValues )
    {
        CAFFA_TRACE( "Setting value from json: " << jsonValue.dump() );

        typename FieldType::FieldDataType value = jsonValue.get<typename FieldType::FieldDataType>();
        m_field->setValue( value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
void FieldIoCap<FieldType>::readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const
{
    this->assertValid();
    if ( copyDataValues )
    {
        jsonValue = m_field->value();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::writeToField( const nlohmann::json& jsonObject,
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

        uint64_t serverAddress = 0u;
        auto     it            = jsonObject.find( "serverAddress" );
        if ( it != jsonObject.end() ) serverAddress = it->get<uint64_t>();

        auto obj = objectFactory->create( className, serverAddress );
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
            obj->setAsParentField( m_field );
            m_field->m_fieldValue.setRawPtr( obj.release() );
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
    ObjectJsonCapability::readFieldsFronJson( objPtr.p(), jsonObject, objectFactory, copyDataValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildField<DataType*>>::readFromField( nlohmann::json& jsonValue,
                                                       bool            copyServerAddress,
                                                       bool            copyDataValues ) const
{
    auto object = m_field->m_fieldValue.rawPtr();
    if ( !object ) return;

    auto ioObject = object->template capability<caffa::ObjectIoCapability>();
    if ( ioObject )
    {
        std::string className = ioObject->classKeyword();

        nlohmann::json jsonObject  = nlohmann::json::object();
        jsonObject["classKeyword"] = className.c_str();
        if ( copyServerAddress )
        {
            jsonObject["serverAddress"] = reinterpret_cast<uint64_t>( object );
        }
        ObjectJsonCapability::writeFieldsToJson( object, jsonObject, copyServerAddress, copyDataValues );
        CAFFA_ASSERT( jsonObject.is_object() );
        jsonValue = jsonObject;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::writeToField( const nlohmann::json& jsonValue,
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

        uint64_t serverAddress = 0u;
        auto     it            = jsonObject.find( "serverAddress" );
        if ( it != jsonObject.end() ) serverAddress = it->get<uint64_t>();

        CAFFA_ASSERT( objectFactory );
        std::unique_ptr<ObjectHandle> obj = objectFactory->create( className, serverAddress );

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

        ObjectJsonCapability::readFieldsFronJson( obj.get(), jsonObject, objectFactory, copyDataValues );

        m_field->m_pointers.push_back( Pointer<DataType>() );
        obj->setAsParentField( m_field );
        m_field->m_pointers.back().setRawPtr( obj.release() );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<ChildArrayField<DataType*>>::readFromField( nlohmann::json& jsonValue,
                                                            bool            copyServerAddress,
                                                            bool            copyDataValues ) const
{
    typename std::vector<Pointer<DataType>>::iterator it;

    nlohmann::json jsonArray = nlohmann::json::array();

    for ( it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it )
    {
        if ( it->rawPtr() == nullptr ) continue;

        auto ioObject = it->rawPtr()->template capability<caffa::ObjectIoCapability>();
        if ( ioObject )
        {
            std::string    className   = ioObject->classKeyword();
            nlohmann::json jsonObject  = nlohmann::json::object();
            jsonObject["classKeyword"] = className;
            if ( copyServerAddress )
            {
                jsonObject["serverAddress"] = reinterpret_cast<uint64_t>( it->rawPtr() );
            }
            ObjectJsonCapability::writeFieldsToJson( it->rawPtr(), jsonObject, copyServerAddress, copyDataValues );
            jsonArray.push_back( jsonObject );
        }
    }
    jsonValue = jsonArray;
}

template <typename DataType>
class JsonConsumer
{
public:
    JsonConsumer( size_t packageCount )
        : m_packageCount( packageCount )
        , m_readCount( 0u )
    {
        m_array = nlohmann::json::array();
    }
    bool consume( std::vector<DataType>&& package )
    {
        if ( package.empty() )
        {
            return true;
        }
        for ( auto item : package )
        {
            m_array.push_back( item );
        }
        return m_readCount++ >= m_packageCount;
    }

    size_t m_packageCount;
    size_t m_readCount;

    nlohmann::json m_array;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<FifoBlockingField<DataType>>::writeToField( const nlohmann::json& jsonValue,
                                                            ObjectFactory*        objectFactory,
                                                            bool                  copyDataValues )
{
    CAFFA_ASSERT( false && "Cannot write to a Fifo Field" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<FifoBlockingField<DataType>>::readFromField( nlohmann::json& jsonValue,
                                                             bool            copyServerAddress,
                                                             bool            copyDataValues ) const
{
    this->assertValid();
    if ( !m_field->active() )
    {
        jsonValue = nlohmann::json::array();
        return;
    }
    const size_t packageCount = m_readLimit;

    using FifoConsumerT = caffa::FifoConsumer<caffa::FifoBlockingField<DataType>>;
    using JsonConsumerT = JsonConsumer<DataType>;

    JsonConsumerT accumulator( packageCount );

    typename FifoConsumerT::PackageHandlingFunction consumptionFunction =
        std::bind( &JsonConsumerT::consume, &accumulator, std::placeholders::_1 );

    FifoConsumerT consumer( const_cast<FifoBlockingField<DataType>*>( m_field ), packageCount, consumptionFunction );

    std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );
    std::thread           consumerThread( consumerFunction );
    consumerThread.join();

    jsonValue = accumulator.m_array;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<FifoBoundedField<DataType>>::writeToField( const nlohmann::json& jsonValue,
                                                           ObjectFactory*        objectFactory,
                                                           bool                  copyDataValues )
{
    CAFFA_ASSERT( false && "Cannot write to a Fifo Field" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldIoCap<FifoBoundedField<DataType>>::readFromField( nlohmann::json& jsonValue,
                                                            bool            copyServerAddress,
                                                            bool            copyDataValues ) const
{
    this->assertValid();
    if ( !m_field->active() )
    {
        jsonValue = nlohmann::json::array();
        return;
    }
    const size_t packageCount = m_readLimit;

    using FifoConsumerT = caffa::FifoConsumer<caffa::FifoBoundedField<DataType>>;
    using JsonConsumerT = JsonConsumer<DataType>;

    JsonConsumerT accumulator( packageCount );

    typename FifoConsumerT::PackageHandlingFunction consumptionFunction =
        std::bind( &JsonConsumerT::consume, &accumulator, std::placeholders::_1 );

    FifoConsumerT consumer( const_cast<FifoBoundedField<DataType>*>( m_field ), packageCount, consumptionFunction );

    std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );
    std::thread           consumerThread( consumerFunction );
    consumerThread.join();

    jsonValue = accumulator.m_array;
}

} // End namespace caffa
