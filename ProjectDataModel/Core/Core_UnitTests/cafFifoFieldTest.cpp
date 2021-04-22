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

template <typename DataType>
class FifoObject : public caffa::ObjectHandle
{
public:
    FifoObject( size_t packageSize = 1024u )
        : ObjectHandle()
        , m_fifoField( packageSize )
    {
        this->addField( &m_fifoField, "FifoField" );
    }

    caffa::FifoField<DataType> m_fifoField;
};

template <typename DataType>
class Producer
{
public:
    Producer( caffa::FifoField<DataType>* ptrToField, size_t valueCount )
        : m_ptrToField( ptrToField )
        , m_finished( false )
        , m_produced( 0u )
        , m_valueCount( valueCount )
    {
    }
    void produce()
    {
        size_t packageSize = m_ptrToField->packageSize();
        while ( m_produced < m_valueCount )
        {
            std::vector<DataType> package;
            package.reserve( packageSize );
            for ( size_t i = 0; i < packageSize && m_produced < m_valueCount; ++i )
            {
                package.push_back( (DataType)m_produced );
                m_produced++;
            }
            m_ptrToField->push( package );
            // CAFFA_INFO( "Pushed value number: " << m_produced );
        }
        CAFFA_INFO( "Finished produced" );
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

    caffa::FifoField<DataType>* m_ptrToField;
    bool                        m_finished;
    std::mutex                  m_mutex;
    size_t                      m_produced;
    size_t                      m_valueCount;
};

template <typename DataType>
class Consumer
{
public:
    Consumer( caffa::FifoField<DataType>* ptrToField, size_t valueCount )
        : m_ptrToField( ptrToField )
        , m_valueCount( valueCount )
    {
        m_buffer.reserve( m_valueCount );
    }
    void consume()
    {
        while ( m_buffer.size() < m_valueCount )
        {
            auto package = m_ptrToField->pop();
            m_buffer.insert( m_buffer.end(), package.begin(), package.end() );
            // CAFFA_INFO( "Popped value" << m_buffer.size() );
        }
        CAFFA_INFO( "Finished consumer" );
    }

    caffa::FifoField<DataType>* m_ptrToField;
    std::vector<DataType>       m_buffer;
    size_t                      m_valueCount;
};

class TimeStampProducer
{
public:
    TimeStampProducer( caffa::FifoField<TimeStamp>* ptrToField, size_t valueCount )
        : m_ptrToField( ptrToField )
        , m_finished( false )
        , m_produced( 0u )
        , m_valueCount( valueCount )
    {
    }

    void produce()
    {
        size_t packageSize = m_ptrToField->packageSize();

        while ( m_produced < m_valueCount )
        {
            std::vector<TimeStamp> package;
            package.reserve( packageSize );
            // CAFFA_INFO( "Package size: " << packageSize );
            for ( size_t i = 0; i < packageSize; ++i )
            {
                package.push_back( std::chrono::system_clock::now() );
                m_produced++;
                // CAFFA_INFO( "Pushed " << i << ", " << m_produced << " values out of " << m_valueCount );
            }

            m_ptrToField->push( package );
        }
        CAFFA_INFO( "Finished producer with package size: " << packageSize );
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

    caffa::FifoField<TimeStamp>* m_ptrToField;
    bool                         m_finished;
    std::mutex                   m_mutex;
    size_t                       m_produced;
    size_t                       m_valueCount;
};

class TimeStampConsumer
{
public:
    TimeStampConsumer( caffa::FifoField<TimeStamp>* ptrToField, size_t valueCount )
        : m_ptrToField( ptrToField )
        , m_valueCount( valueCount )
    {
        m_buffer.reserve( m_valueCount );
    }
    void consume()
    {
        while ( m_buffer.size() < m_valueCount )
        {
            auto thenPackage = m_ptrToField->pop();
            for ( const auto& then : thenPackage )
            {
                auto now      = std::chrono::system_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>( now - then );
                m_buffer.push_back( duration );
                // CAFFA_INFO( "Latency: " << duration.count() );
            }
        }
        CAFFA_INFO( "Finished consumer" );
    }
    std::array<double, 5> calculateMinMaxAverageLatencyMs() const
    {
        double sumMs = 0.0;
        double maxMs = 0.0;
        double minMs = std::numeric_limits<double>::max();

        std::vector<double> allMs;
        for ( const auto& latency : m_buffer )
        {
            double dblLatency = latency.count();
            maxMs             = std::max( maxMs, dblLatency );
            minMs             = std::min( minMs, dblLatency );

            sumMs += dblLatency;
            allMs.push_back( dblLatency );
        }
        std::sort( allMs.begin(), allMs.end() );

        return { minMs, maxMs, sumMs / m_buffer.size(), allMs[allMs.size() / 2], allMs[99 * allMs.size() / 100] };
    }

    caffa::FifoField<TimeStamp>*           m_ptrToField;
    std::vector<std::chrono::microseconds> m_buffer;
    size_t                                 m_valueCount;
};

TEST( FifoObject, TestUint64_t )
{
    const size_t         valueCount = 1000u;
    FifoObject<uint64_t> object;

    Consumer<uint64_t> consumer( &object.m_fifoField, valueCount );
    Producer<uint64_t> producer( &object.m_fifoField, valueCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<uint64_t>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<uint64_t>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( valueCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestDouble )
{
    const size_t       valueCount = 1000u;
    FifoObject<double> object;

    Consumer<double> consumer( &object.m_fifoField, valueCount );
    Producer<double> producer( &object.m_fifoField, valueCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<double>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<double>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( valueCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestFloat )
{
    const size_t      valueCount = 1000u;
    FifoObject<float> object;

    Consumer<float> consumer( &object.m_fifoField, valueCount );
    Producer<float> producer( &object.m_fifoField, valueCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<float>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<float>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( valueCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestChar )
{
    const size_t     valueCount = 1000u;
    FifoObject<char> object;

    Consumer<char> consumer( &object.m_fifoField, valueCount );
    Producer<char> producer( &object.m_fifoField, valueCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &Producer<char>::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &Consumer<char>::consume, &consumer );

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    ASSERT_EQ( valueCount, consumer.m_buffer.size() );
}

TEST( FifoObject, TestLatency )
{
    size_t valueCount = 1024u * 1024 * 8;
    size_t bytes      = sizeof( TimeStamp ) * valueCount;
    double MiB        = bytes / 1024.0 / 1024.0;
    std::cout << "Total MiB being sent for each test: " << MiB << std::endl;
    FifoObject<TimeStamp> object( 32u );

    TimeStampConsumer consumer( &object.m_fifoField, valueCount );
    TimeStampProducer producer( &object.m_fifoField, valueCount );
    ASSERT_EQ( (size_t)0, consumer.m_buffer.size() );

    std::function<void()> producerFunction = std::bind( &TimeStampProducer::produce, &producer );
    std::function<void()> consumerFunction = std::bind( &TimeStampConsumer::consume, &consumer );

    auto before = std::chrono::system_clock::now();

    std::thread consumerThread( consumerFunction );
    std::thread producerThread( producerFunction );

    consumerThread.join();
    producer.setFinished();
    producerThread.join();

    auto after = std::chrono::system_clock::now();

    ASSERT_EQ( valueCount, consumer.m_buffer.size() );

    auto [min, max, avg, med, q99] = consumer.calculateMinMaxAverageLatencyMs();

    size_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>( after - before ).count();

    std::cout << "---- Timings with ----" << std::endl;
    std::cout << "Received values: " << valueCount << std::endl;
    std::cout << "Dropped values: " << object.m_fifoField.droppedValues() << std::endl;
    std::cout << "Total time: " << microseconds << " μs" << std::endl;
    std::cout << "Mbit/s: " << MiB * 8 * 1000 * 1000 / microseconds << std::endl;
    std::cout << "Latencies -> min: " << min << " μs, avg: " << avg << " μs, med: " << med << " μs, Q99: " << q99
              << " μs, max: " << max << " μs" << std::endl;
}
