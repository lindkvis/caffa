
#include "gtest.h"

#include "cafFifoField.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"

using TimeStamp = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;

class MyFifoObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    MyFifoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_boundedField, "BoundedField" );
        CAFFA_IO_InitField( &m_blockingField, "BlockingField" );
    }

    ~MyFifoObject() {}

    // Fields
    caffa::FifoBoundedField<float>  m_boundedField;
    caffa::FifoBlockingField<float> m_blockingField;
};

CAFFA_IO_SOURCE_INIT( MyFifoObject, "MyFifoObject" );

template <typename DataType>
class TestCreator
{
    using Package = std::vector<DataType>;

public:
    TestCreator( size_t packageSize )
        : m_packageSize( packageSize )
    {
    }
    Package makeValue( size_t packageIndex )
    {
        Package package;
        package.reserve( m_packageSize );
        for ( size_t i = 0; i < m_packageSize; ++i )
        {
            package.push_back( (DataType)1 );
        }
        return package;
    }

    size_t m_packageSize;
};

template <>
class TestCreator<TimeStamp>
{
    using Package = std::vector<TimeStamp>;

public:
    TestCreator( size_t packageSize )
        : m_packageSize( packageSize )
    {
    }
    Package makeValue( size_t packageIndex )
    {
        Package package;
        package.reserve( m_packageSize );
        for ( size_t i = 0; i < m_packageSize; ++i )
        {
            package.push_back( std::chrono::system_clock::now() );
        }

        return package;
    }
    size_t m_packageSize;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( FifoTest, SerializeBoundedFloatFifo )
{
    MyFifoObject object;

    using DataType = float;

    const size_t packageCount = 50;
    using TestCreatorT        = TestCreator<DataType>;
    using FifoProducerT       = caffa::FifoProducer<DataType>;

    TestCreatorT creator( 4u );

    auto ioCap = object.m_boundedField.capability<caffa::FieldIoCap<caffa::FifoBoundedField<DataType>>>();
    ASSERT_TRUE( ioCap != nullptr );
    ioCap->setMaximumPackagesForIoOutput( packageCount );

    typename FifoProducerT::PackageCreatorFunction creatorFunction =
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );

    FifoProducerT producer( &object.m_boundedField, packageCount, creatorFunction );

    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );

    std::thread producerThread( producerFunction );

    auto serializedString = object.writeObjectToString( caffa::ObjectIoCapability::IoType::JSON );

    std::cout << "Serialized Object: " << serializedString << std::endl;

    producer.setFinished();
    producerThread.join();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( FifoTest, SerializeBlockingFloatFifo )
{
    MyFifoObject object;

    using DataType = float;

    const size_t packageCount = 50;
    using TestCreatorT        = TestCreator<DataType>;
    using FifoProducerT       = caffa::FifoProducer<DataType>;

    TestCreatorT creator( 4u );

    auto ioCap = object.m_blockingField.capability<caffa::FieldIoCap<caffa::FifoBlockingField<DataType>>>();
    ASSERT_TRUE( ioCap != nullptr );
    ioCap->setMaximumPackagesForIoOutput( packageCount );

    typename FifoProducerT::PackageCreatorFunction creatorFunction =
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );

    FifoProducerT producer( &object.m_blockingField, packageCount, creatorFunction );

    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );

    std::thread producerThread( producerFunction );
    auto        serializedString = object.writeObjectToString( caffa::ObjectIoCapability::IoType::JSON );

    std::cout << serializedString << std::endl;

    producer.setFinished();
    producerThread.join();
}
