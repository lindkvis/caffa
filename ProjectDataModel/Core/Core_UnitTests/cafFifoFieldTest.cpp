#include "gtest.h"

#include "cafFifoField.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"
#include "cafPortableDataType.h"

#include <array>
#include <chrono>
#include <deque>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using TimeStamp = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;

namespace caffa
{
CAFFA_DEFINE_PORTABLE_TYPE_NAME( TimeStamp, "TimeStamp" );
}

template <typename DataType>
class FifoObject : public caffa::ObjectHandle
{
public:
    FifoObject( size_t bufferSize, size_t packageSize )
        : ObjectHandle()
        , m_boundedField( bufferSize, packageSize )
        , m_blockingField( bufferSize, packageSize )
    {
        this->addField( &m_boundedField, "FifoBoundedField" );
        this->addField( &m_blockingField, "FifoBlockingField" );
    }

    caffa::FifoBoundedField<DataType>  m_boundedField;
    caffa::FifoBlockingField<DataType> m_blockingField;
};

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

template <typename DataType>
class TestConsumer
{
public:
    TestConsumer( size_t packageCount )
        : m_packageCount( packageCount )
    {
        m_buffer.reserve( packageCount );
    }
    bool consume( std::vector<DataType>&& package )
    {
        m_buffer.push_back( std::move( package ) );
        return m_buffer.size() >= m_packageCount;
    }

    size_t                             m_packageCount;
    std::vector<std::vector<DataType>> m_buffer;
};

template <>
class TestConsumer<TimeStamp>
{
public:
    TestConsumer( size_t packageCount )
        : m_packageCount( packageCount )
    {
        m_buffer.reserve( packageCount );
    }

    bool consume( std::vector<TimeStamp>&& package )
    {
        auto epoch = std::chrono::time_point<std::chrono::system_clock>{};
        for ( TimeStamp& then : package )
        {
            auto now      = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( now - then );
            then          = epoch + duration;
            // CAFFA_TRACE( "Time stamp: " << duration.count() );
        }
        m_buffer.push_back( std::move( package ) );
        return m_buffer.size() >= m_packageCount;
    }

    size_t                              m_packageCount;
    std::vector<std::vector<TimeStamp>> m_buffer;
};

std::array<double, 5> calculateMinMaxAverageLatencyMs( const std::vector<std::vector<TimeStamp>>& buffer )
{
    auto epoch = std::chrono::time_point<std::chrono::system_clock>{};

    double sumMs = 0.0;
    double maxMs = 0.0;
    double minMs = std::numeric_limits<double>::max();

    std::vector<double> allMs;
    for ( const auto& package : buffer )
    {
        for ( const TimeStamp& latency : package )
        {
            double dblLatency =
                static_cast<double>( std::chrono::duration_cast<std::chrono::microseconds>( latency - epoch ).count() );
            maxMs = std::max( maxMs, dblLatency );
            minMs = std::min( minMs, dblLatency );

            sumMs += dblLatency;
            allMs.push_back( dblLatency );
        }
    }
    std::sort( allMs.begin(), allMs.end() );

    return { minMs, maxMs, sumMs / allMs.size(), allMs[allMs.size() / 2], allMs[99 * allMs.size() / 100] };
}

#define RUN_SIMPLE_FIFO_TEST( DataType, FieldType, FieldName, PACKAGE_COUNT, BUFFER_SIZE, PACKAGE_SIZE ) \
    const size_t packageCount = PACKAGE_COUNT;                                                           \
    using FifoObjectT         = FifoObject<DataType>;                                                    \
    using TestCreatorT        = TestCreator<DataType>;                                                   \
    using TestConsumerT       = TestConsumer<DataType>;                                                  \
                                                                                                         \
    using FifoProducerT = caffa::FifoProducer<FieldType<DataType>>;                                      \
    using FifoConsumerT = caffa::FifoConsumer<FieldType<DataType>>;                                      \
                                                                                                         \
    FifoObjectT   object( BUFFER_SIZE, PACKAGE_SIZE );                                                   \
    TestCreatorT  creator( PACKAGE_SIZE );                                                               \
    TestConsumerT accumulator( packageCount );                                                           \
                                                                                                         \
    typename FifoProducerT::PackageCreatorFunction creatorFunction =                                     \
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );                          \
                                                                                                         \
    typename FifoConsumerT::PackageHandlingFunction consumptionFunction =                                \
        std::bind( &TestConsumerT::consume, &accumulator, std::placeholders::_1 );                       \
                                                                                                         \
    FifoConsumerT consumer( &object.FieldName, packageCount, consumptionFunction );                      \
    FifoProducerT producer( &object.FieldName, packageCount, creatorFunction );                          \
    ASSERT_EQ( (size_t)0, accumulator.m_buffer.size() );                                                 \
                                                                                                         \
    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );            \
    std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );            \
                                                                                                         \
    std::thread producerThread( producerFunction );                                                      \
    std::thread consumerThread( consumerFunction );                                                      \
                                                                                                         \
    consumerThread.join();                                                                               \
    producer.setFinished();                                                                              \
    producerThread.join();                                                                               \
                                                                                                         \
    ASSERT_EQ( packageCount, accumulator.m_buffer.size() );

TEST( FifoObject, TestUint64_t )
{
    {
        RUN_SIMPLE_FIFO_TEST( uint64_t, caffa::FifoBoundedField, m_boundedField, 10000u, 8u, 32u );
    }
    {
        RUN_SIMPLE_FIFO_TEST( uint64_t, caffa::FifoBlockingField, m_blockingField, 10000u, 16u, 64u );
    }
}

TEST( FifoObject, TestDouble )
{
    {
        RUN_SIMPLE_FIFO_TEST( double, caffa::FifoBoundedField, m_boundedField, 10000u, 8u, 32u );
    }
    {
        RUN_SIMPLE_FIFO_TEST( double, caffa::FifoBoundedField, m_boundedField, 10000u, 12u, 24u );
    }
}

TEST( FifoObject, TestFloat )
{
    {
        RUN_SIMPLE_FIFO_TEST( float, caffa::FifoBoundedField, m_boundedField, 10000u, 8u, 32u );
    }
    {
        RUN_SIMPLE_FIFO_TEST( float, caffa::FifoBlockingField, m_blockingField, 10000u, 9u, 15u );
    }
}

TEST( FifoObject, TestChar )
{
    {
        RUN_SIMPLE_FIFO_TEST( float, caffa::FifoBlockingField, m_blockingField, 10000u, 8u, 32u );
    }
    {
        RUN_SIMPLE_FIFO_TEST( float, caffa::FifoBoundedField, m_boundedField, 10000u, 10u, 5u );
    }
}

TEST( FifoObject, TestLatencyBounded )
{
    auto before = std::chrono::system_clock::now();
    RUN_SIMPLE_FIFO_TEST( TimeStamp, caffa::FifoBoundedField, m_boundedField, 1024u * 256u, 16u, 32u );
    auto after = std::chrono::system_clock::now();

    auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( accumulator.m_buffer );

    size_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>( after - before ).count();

    size_t bytes = sizeof( TimeStamp ) * packageCount * object.m_boundedField.packageSize();
    double MiB   = bytes / 1024.0 / 1024.0;

    std::cout << "---- Timings with ----" << std::endl;
    std::cout << "Received packages: " << packageCount << std::endl;
    std::cout << "Dropped packages: " << object.m_boundedField.droppedPackages() << std::endl;
    std::cout << "Total time: " << microseconds << " μs" << std::endl;
    std::cout << "Mbit/s: " << MiB * 8 * 1000 * 1000 / microseconds << std::endl;
    std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
              << " μs, max: " << max << " μs" << std::endl;
}

TEST( FifoObject, TestLatencyBlocking )
{
    auto before = std::chrono::system_clock::now();
    RUN_SIMPLE_FIFO_TEST( TimeStamp, caffa::FifoBlockingField, m_blockingField, 1024u * 256u, 8u, 32u );
    auto after = std::chrono::system_clock::now();

    auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( accumulator.m_buffer );

    size_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>( after - before ).count();

    size_t bytes = sizeof( TimeStamp ) * packageCount * object.m_blockingField.packageSize();
    double MiB   = bytes / 1024.0 / 1024.0;

    std::cout << "---- Timings with ----" << std::endl;
    std::cout << "Received packages: " << packageCount << std::endl;
    std::cout << "Dropped packages: " << object.m_blockingField.droppedPackages() << std::endl;
    std::cout << "Total time: " << microseconds << " μs" << std::endl;
    std::cout << "Mbit/s: " << MiB * 8 * 1000 * 1000 / microseconds << std::endl;
    std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
              << " μs, max: " << max << " μs" << std::endl;
}

TEST( FifoObject, MultipleAccessToBounded )
{
    const size_t packageCount = 2000u;
    const size_t BUFFER_SIZE  = 8u;
    const size_t PACKAGE_SIZE = 32u;

    using DataType = TimeStamp;

    using FifoObjectT   = FifoObject<DataType>;
    using TestCreatorT  = TestCreator<DataType>;
    using TestConsumerT = TestConsumer<DataType>;

    using FifoProducerT = caffa::FifoProducer<caffa::FifoBoundedField<DataType>>;
    using FifoConsumerT = caffa::FifoConsumer<caffa::FifoBoundedField<DataType>>;

    FifoObjectT  object( BUFFER_SIZE, PACKAGE_SIZE );
    TestCreatorT creator( PACKAGE_SIZE );

    typename FifoProducerT::PackageCreatorFunction creatorFunction =
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );
    FifoProducerT         producer( &object.m_boundedField, std::numeric_limits<size_t>::max(), creatorFunction );
    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );
    std::thread           producerThread( producerFunction );

    // Read multiple times, and sleep in between. The producer should keep on producing but dropping packages.
    // ... meaning the latencies shouldn't increase dramatically because when we start reading again we should
    // ... be getting new data
    size_t                     totalReadCount = 0u;
    const size_t               repeats        = 200u;
    std::vector<TestConsumerT> accumulator( repeats, TestConsumerT( packageCount ) );
    for ( size_t i = 0; i < repeats; ++i )
    {
        typename FifoConsumerT::PackageHandlingFunction consumptionFunction =
            std::bind( &TestConsumerT::consume, &accumulator[i], std::placeholders::_1 );
        FifoConsumerT consumer( &object.m_boundedField, packageCount, consumptionFunction );
        ASSERT_EQ( (size_t)0, accumulator[i].m_buffer.size() );
        std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );

        std::thread consumerThread( consumerFunction );
        consumerThread.join();
        ASSERT_EQ( packageCount, accumulator[i].m_buffer.size() );
        totalReadCount += accumulator[i].m_buffer.size();
    }
    std::cout << "Produced a total of " << producer.productionCount() << " packages but only read " << totalReadCount
              << std::endl;

    for ( size_t i = 0; i < repeats; ++i )
    {
        auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( accumulator[i].m_buffer );
        std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
                  << " μs, max: " << max << " μs" << std::endl;
    }
    ASSERT_LT( totalReadCount, producer.productionCount() );

    producer.setFinished();
    producerThread.join();
}

TEST( FifoObject, MultipleAccessToBoundedLargerBuffer )
{
    const size_t packageCount = 2000u;
    const size_t BUFFER_SIZE  = 512u;
    const size_t PACKAGE_SIZE = 32u;

    using DataType = TimeStamp;

    using FifoObjectT   = FifoObject<DataType>;
    using TestCreatorT  = TestCreator<DataType>;
    using TestConsumerT = TestConsumer<DataType>;

    using FifoProducerT = caffa::FifoProducer<caffa::FifoBoundedField<DataType>>;
    using FifoConsumerT = caffa::FifoConsumer<caffa::FifoBoundedField<DataType>>;

    FifoObjectT  object( BUFFER_SIZE, PACKAGE_SIZE );
    TestCreatorT creator( PACKAGE_SIZE );

    typename FifoProducerT::PackageCreatorFunction creatorFunction =
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );
    FifoProducerT         producer( &object.m_boundedField, std::numeric_limits<size_t>::max(), creatorFunction );
    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );
    std::thread           producerThread( producerFunction );

    // Read multiple times, and sleep in between. The producer should keep on producing but dropping packages.
    // ... meaning the latencies shouldn't increase dramatically because when we start reading again we should
    // ... be getting new data
    size_t totalReadCount = 0u;
    for ( size_t i = 0; i < 10; ++i )
    {
        TestConsumerT                                   accumulator( packageCount );
        typename FifoConsumerT::PackageHandlingFunction consumptionFunction =
            std::bind( &TestConsumerT::consume, &accumulator, std::placeholders::_1 );
        FifoConsumerT consumer( &object.m_boundedField, packageCount, consumptionFunction );
        ASSERT_EQ( (size_t)0, accumulator.m_buffer.size() );
        std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );

        std::thread consumerThread( consumerFunction );
        consumerThread.join();
        ASSERT_EQ( packageCount, accumulator.m_buffer.size() );
        totalReadCount += accumulator.m_buffer.size();
        auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( accumulator.m_buffer );
        std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
                  << " μs, max: " << max << " μs" << std::endl;
    }
    std::cout << "Produced a total of " << producer.productionCount() << " packages but only read " << totalReadCount
              << std::endl;
    ASSERT_LT( totalReadCount, producer.productionCount() );

    producer.setFinished();
    producerThread.join();
}

TEST( FifoObject, MultipleAccessToBlocked )
{
    const size_t packageCount = 2000u;
    const size_t BUFFER_SIZE  = 8u;
    const size_t PACKAGE_SIZE = 32u;

    using DataType = TimeStamp;

    using FifoObjectT   = FifoObject<DataType>;
    using TestCreatorT  = TestCreator<DataType>;
    using TestConsumerT = TestConsumer<DataType>;

    using FifoProducerT = caffa::FifoProducer<caffa::FifoBlockingField<DataType>>;
    using FifoConsumerT = caffa::FifoConsumer<caffa::FifoBlockingField<DataType>>;

    FifoObjectT  object( BUFFER_SIZE, PACKAGE_SIZE );
    TestCreatorT creator( PACKAGE_SIZE );

    typename FifoProducerT::PackageCreatorFunction creatorFunction =
        std::bind( &TestCreatorT::makeValue, &creator, std::placeholders::_1 );
    FifoProducerT         producer( &object.m_blockingField, std::numeric_limits<size_t>::max(), creatorFunction );
    std::function<void()> producerFunction = std::bind( &FifoProducerT::produce, &producer );
    std::thread           producerThread( producerFunction );

    // Read multiple times. The producer should block meaning we shouldn't drop any packages
    // ... and the max latencies will increase as one full buffer will wait for the next consumer
    size_t totalReadCount = 0u;
    for ( size_t i = 0; i < 10; ++i )
    {
        TestConsumerT                                   accumulator( packageCount );
        typename FifoConsumerT::PackageHandlingFunction consumptionFunction =
            std::bind( &TestConsumerT::consume, &accumulator, std::placeholders::_1 );
        FifoConsumerT consumer( &object.m_blockingField, packageCount, consumptionFunction );
        ASSERT_EQ( (size_t)0, accumulator.m_buffer.size() );
        std::function<void()> consumerFunction = std::bind( &FifoConsumerT::consume, &consumer );

        std::thread consumerThread( consumerFunction );
        consumerThread.join();
        ASSERT_EQ( packageCount, accumulator.m_buffer.size() );
        totalReadCount += accumulator.m_buffer.size();
        auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( accumulator.m_buffer );
        std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
                  << " μs, max: " << max << " μs" << std::endl;
    }
    // Since this is a blocking field we should only have produced the amount read + the size of the buffer.
    std::cout << "Produced a total of " << producer.productionCount() << " packages and read " << totalReadCount
              << std::endl;
    ASSERT_LE( producer.validProductionCount(), totalReadCount + 4 * BUFFER_SIZE );

    ASSERT_LE( totalReadCount, producer.validProductionCount() );

    producer.setFinished();
    producerThread.join();
}