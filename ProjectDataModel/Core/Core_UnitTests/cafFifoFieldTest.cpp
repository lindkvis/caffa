#include "gtest.h"

#include "cafFifoField.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"

#include <array>
#include <chrono>
#include <deque>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using TimeStamp = std::chrono::time_point<std::chrono::system_clock>;

template <typename DataType, size_t BUFFER_SIZE = 8u, size_t PACKAGE_SIZE = 32u>
class FifoObject : public caffa::ObjectHandle
{
public:
    static const size_t bufferSize  = BUFFER_SIZE;
    static const size_t packageSize = PACKAGE_SIZE;

    FifoObject()
        : ObjectHandle()
        , m_fifoField()
    {
        this->addField( &m_fifoField, "FifoField" );
    }

    caffa::FifoField<DataType, BUFFER_SIZE, PACKAGE_SIZE> m_fifoField;
};

template <typename DataType>
DataType makeValue()
{
    return (DataType)1;
}

template <>
TimeStamp makeValue<TimeStamp>()
{
    return std::chrono::system_clock::now();
}

template <typename DataType, size_t BUFFER_SIZE = 8u, size_t PACKAGE_SIZE = 32u>
class Producer
{
    using FifoField = caffa::FifoField<DataType, BUFFER_SIZE, PACKAGE_SIZE>;

public:
    Producer( FifoField* ptrToField, size_t sweepCount )
        : m_ptrToField( ptrToField )
        , m_finished( false )
        , m_doneCount( 0u )
        , m_sweepCount( sweepCount )
    {
    }
    void produce()
    {
        while ( m_doneCount - m_ptrToField->droppedPackages() < m_sweepCount )
        {
            std::array<DataType, PACKAGE_SIZE> package;
            for ( size_t i = 0; i < PACKAGE_SIZE; ++i )
            {
                package[i] = makeValue<DataType>();
            }
            // CAFFA_INFO( "Pushed value number: " << m_doneCount - m_ptrToField->droppedPackages() << " out of "
            //                                  << m_sweepCount );

            m_ptrToField->push( package );
            m_doneCount++;
        }
        // CAFFA_INFO( "Finished producing" );
    }

    bool finished()
    {
        std::unique_lock<std::mutex>( m_mutex );
        return m_finished;
    }
    void setFinished()
    {
        std::unique_lock<std::mutex>( m_mutex );
        m_finished = true;
    }

    FifoField* m_ptrToField;
    bool       m_finished;
    std::mutex m_mutex;
    size_t     m_doneCount;
    size_t     m_sweepCount;
};

template <typename DataType, size_t PACKAGE_SIZE>
class Evaluator
{
public:
    static std::array<DataType, PACKAGE_SIZE> evaluatePackage( std::array<DataType, PACKAGE_SIZE>& package )
    {
        return std::move( package );
    }
};

template <size_t PACKAGE_SIZE>
class Evaluator<TimeStamp, PACKAGE_SIZE>
{
public:
    static std::array<TimeStamp, PACKAGE_SIZE> evaluatePackage( std::array<TimeStamp, PACKAGE_SIZE>& package )
    {
        auto epoch = std::chrono::time_point<std::chrono::system_clock>{};
        for ( TimeStamp& then : package )
        {
            auto now      = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( now - then );
            then          = epoch + duration;
            // CAFFA_TRACE( "Time stamp: " << duration.count() );
        }
        return std::move( package );
    }
};

template <typename DataType, size_t BUFFER_SIZE = 8u, size_t PACKAGE_SIZE = 32u>
class Consumer
{
    using FifoField = caffa::FifoField<DataType, BUFFER_SIZE, PACKAGE_SIZE>;

public:
    Consumer( FifoField* ptrToField, size_t sweepCount )
        : m_ptrToField( ptrToField )
        , m_sweepCount( sweepCount )
    {
        m_buffer.reserve( m_sweepCount );
    }
    void consume()
    {
        while ( m_buffer.size() < m_sweepCount )
        {
            // CAFFA_INFO( "Trying to pop value" << m_buffer.size() );
            auto package = m_ptrToField->pop();
            // CAFFA_INFO( "Popped value" << m_buffer.size() );

            m_buffer.push_back( Evaluator<DataType, PACKAGE_SIZE>::evaluatePackage( package ) );
        }
        // CAFFA_INFO( "Finished consumer" );
    }

    FifoField*                                      m_ptrToField;
    std::vector<std::array<DataType, PACKAGE_SIZE>> m_buffer;
    size_t                                          m_sweepCount;
};

template <size_t PACKAGE_SIZE>
std::array<double, 5> calculateMinMaxAverageLatencyMs( const std::vector<std::array<TimeStamp, PACKAGE_SIZE>>& buffer )
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
            double dblLatency = std::chrono::duration_cast<std::chrono::microseconds>( latency - epoch ).count();
            maxMs             = std::max( maxMs, dblLatency );
            minMs             = std::min( minMs, dblLatency );

            sumMs += dblLatency;
            allMs.push_back( dblLatency );
        }
    }
    std::sort( allMs.begin(), allMs.end() );

    return { minMs, maxMs, sumMs / allMs.size(), allMs[allMs.size() / 2], allMs[99 * allMs.size() / 100] };
}

TEST( FifoObject, TestUint64_t )
{
    const size_t         packageCount = 1000u;
    FifoObject<uint64_t> object;

    Consumer<uint64_t> consumer( &object.m_fifoField, packageCount );
    Producer<uint64_t> producer( &object.m_fifoField, packageCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<uint64_t>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<uint64_t>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( packageCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestDouble )
{
    const size_t       packageCount = 1000u;
    FifoObject<double> object;

    Consumer<double> consumer( &object.m_fifoField, packageCount );
    Producer<double> producer( &object.m_fifoField, packageCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<double>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<double>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( packageCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestFloat )
{
    const size_t      packageCount = 1000u;
    FifoObject<float> object;

    Consumer<float> consumer( &object.m_fifoField, packageCount );
    Producer<float> producer( &object.m_fifoField, packageCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<float>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<float>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( packageCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestChar )
{
    const size_t     packageCount = 1000u;
    FifoObject<char> object;

    Consumer<char> consumer( &object.m_fifoField, packageCount );
    Producer<char> producer( &object.m_fifoField, packageCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<char>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<char>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( packageCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestLatency )
{
#define BUFFER_SIZE 8u
#define PACKAGE_SIZE 24u

    using FifoObject = FifoObject<TimeStamp, BUFFER_SIZE, PACKAGE_SIZE>;
    using Consumer   = Consumer<TimeStamp, BUFFER_SIZE, PACKAGE_SIZE>;
    using Producer   = Producer<TimeStamp, BUFFER_SIZE, PACKAGE_SIZE>;

    size_t packageCount = 1024u * 1024 * 1;
    size_t bytes        = sizeof( TimeStamp ) * packageCount * FifoObject::packageSize;
    double MiB          = bytes / 1024.0 / 1024.0;
    std::cout << "Total MiB being sent: " << MiB << std::endl;

    FifoObject object;
    Consumer   consumer( &object.m_fifoField, packageCount );
    Producer   producer( &object.m_fifoField, packageCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer::consume, &consumer );

    auto before = std::chrono::system_clock::now();

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    auto after = std::chrono::system_clock::now();

    ASSERT_EQ( packageCount, consumer.m_buffer.size() );

    auto [min, max, avg, med, q99] = calculateMinMaxAverageLatencyMs( consumer.m_buffer );

    size_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>( after - before ).count();

    std::cout << "---- Timings with ----" << std::endl;
    std::cout << "Received packages: " << packageCount << std::endl;
    std::cout << "Dropped packages: " << object.m_fifoField.droppedPackages() << std::endl;
    std::cout << "Total time: " << microseconds << " μs" << std::endl;
    std::cout << "Mbit/s: " << MiB * 8 * 1000 * 1000 / microseconds << std::endl;
    std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
              << " μs, max: " << max << " μs" << std::endl;
}
