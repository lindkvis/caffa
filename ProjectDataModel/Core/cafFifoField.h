//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafPortableDataType.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace caffa
{
/**
 * Templated FIFO (First in, First Out) circular buffer field. Blocking.
 */
template <typename DataType>
class FifoBlockingField : public caffa::FieldHandle
{
public:
    using FieldDataType = DataType;
    using Package       = std::vector<FieldDataType>;
    using Queue         = std::vector<Package>;

    FifoBlockingField( size_t packageCount = 8u, size_t packageSize = 32u )
        : m_packageCount( packageCount )
        , m_packageSize( packageSize )
        , m_readIndex( 0u )
        , m_writeIndex( 0u )
        , m_active( false )
    {
        m_buffer.resize( m_packageCount, Package( m_packageSize ) );
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        m_readIndex  = 0u;
        m_writeIndex = 0u;
    }

    size_t packageSize() const { return m_packageSize; }

    bool active() const
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        return m_active;
    }
    void setActive( bool active )
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        m_active = active;
    }
    std::optional<Package> pop()
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 100ms, [this]() { return !this->empty(); } );

        if ( !this->empty() )
        {
            Package package( this->m_packageSize );
            package.swap( this->m_buffer[this->m_readIndex++ % this->m_packageCount] );
            lock.unlock();
            this->m_dataAccessGuard.notify_one();
            return package;
        }

        lock.unlock();
        this->m_dataAccessGuard.notify_one();
        return std::nullopt;
    }

    void push( Package& package )
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 100ms, [this]() { return !this->full(); } );
        if ( !this->full() )
        {
            this->m_buffer[this->m_writeIndex++ % this->m_packageCount].swap( package );
            lock.unlock();
            this->m_dataAccessGuard.notify_one();
        }
    }
    size_t droppedPackages() const { return 0u; }

    std::string dataType() const override { return PortableDataType<DataType>::name(); }

private:
    bool empty() const
    {
        // CAFFA_TRACE( "Indices: " << m_writeIndex << ", " << m_readIndex );
        return m_writeIndex <= m_readIndex;
    }
    bool full() const { return m_writeIndex - m_readIndex >= m_packageCount; }

private:
    bool  m_active;
    Queue m_buffer;

    size_t m_packageCount;
    size_t m_packageSize;

    size_t m_readIndex;
    size_t m_writeIndex;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;
};

/**
 * Templated FIFO (First in, First Out) circular buffer field. Non-Blocking. Drops packages when full.
 */
template <typename DataType>
class FifoBoundedField : public caffa::FieldHandle
{
public:
    using FieldDataType = DataType;
    using Package       = std::vector<FieldDataType>;
    using Queue         = std::vector<Package>;

    FifoBoundedField( size_t packageCount = 8u, size_t packageSize = 32u )
        : m_packageCount( packageCount )
        , m_packageSize( packageSize )
        , m_bufferSize( packageCount * packageSize )
        , m_readIndex( 0u )
        , m_writeIndex( 0u )
        , m_active( false )
        , m_droppedPackages( 0u )
    {
        m_buffer.resize( m_bufferSize );
        // CAFFA_WARNING( "Buffer size does not match page size: " << m_bufferSize << ", " << getpagesize() );
    }
    void clear()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        m_readIndex  = 0u;
        m_writeIndex = 0u;
    }

    size_t packageSize() const { return m_packageSize; }

    bool active() const
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        return m_active;
    }
    void setActive( bool active )
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        m_active = active;
    }

    std::optional<Package> pop()
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 100ms, [this]() { return !this->empty(); } );

        if ( !this->empty() )
        {
            Package package( this->m_packageSize );
            package.swap( this->m_buffer[this->m_readIndex++ % this->m_packageCount] );
            lock.unlock();
            this->m_dataAccessGuard.notify_one();
            return package;
        }

        lock.unlock();
        this->m_dataAccessGuard.notify_one();
        return std::nullopt;
    }

    void push( Package& package )
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        if ( this->full() )
        {
            this->m_readIndex += 1;
            this->m_droppedPackages += 1;
        }

        this->m_buffer[this->m_writeIndex++ % this->m_packageCount].swap( package );
        lock.unlock();
        this->m_dataAccessGuard.notify_one();
    }

    size_t droppedPackages() const
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        return this->m_droppedPackages;
    }

    std::string dataType() const override { return PortableDataType<DataType>::name(); }

private:
    bool empty() const
    {
        // CAFFA_TRACE( "Indices: " << m_writeIndex << ", " << m_readIndex );
        return m_writeIndex <= m_readIndex;
    }
    bool full() const { return m_writeIndex - m_readIndex >= m_packageCount; }

private:
    bool  m_active;
    Queue m_buffer;

    size_t m_packageCount;
    size_t m_packageSize;
    size_t m_bufferSize;

    size_t m_readIndex;
    size_t m_writeIndex;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;

    size_t m_droppedPackages;
};

template <typename ProductionField>
class FifoProducer
{
public:
    using Package = typename ProductionField::Package;

    // Package index as the argument
    using PackageCreatorFunction = std::function<Package( size_t )>;

public:
    FifoProducer( ProductionField* ptrToField, size_t sweepCount, PackageCreatorFunction packageCreator )
        : m_ptrToField( ptrToField )
        , m_finished( false )
        , m_doneCount( 0u )
        , m_sweepCount( sweepCount )
        , m_packageCreator( packageCreator )
    {
        m_ptrToField->setActive( true );
    }
    void produce()
    {
        while ( ( m_sweepCount == std::numeric_limits<size_t>::infinity() || validProductionCount() < m_sweepCount ) &&
                !finished() )
        {
            Package package = m_packageCreator( m_doneCount );
            // CAFFA_INFO( "Pushed value number: " << m_doneCount - m_ptrToField->droppedPackages() << " out of "
            //                                  << m_sweepCount );

            m_ptrToField->push( package );
            m_doneCount++;
        }
        m_ptrToField->setActive( false );
    }
    size_t validProductionCount() const { return m_doneCount - m_ptrToField->droppedPackages(); }
    size_t productionCount() const { return m_doneCount; }

    bool finished()
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        return m_finished;
    }
    void setFinished()
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        m_finished = true;
    }

    ProductionField*       m_ptrToField;
    bool                   m_finished;
    std::mutex             m_mutex;
    size_t                 m_doneCount;
    size_t                 m_sweepCount;
    PackageCreatorFunction m_packageCreator;
};

template <typename ConsumptionField>
class FifoConsumer
{
public:
    using Package                 = typename ConsumptionField::Package;
    using PackageHandlingFunction = std::function<bool( Package&& )>;

public:
    FifoConsumer( ConsumptionField* ptrToField, size_t sweepCount, PackageHandlingFunction packageHandlingFunction )
        : m_ptrToField( ptrToField )
        , m_sweepCount( sweepCount )
        , m_consumedCount( 0u )
        , m_packageHandlingFunction( packageHandlingFunction )
    {
    }
    void consume()
    {
        size_t failedPops = 0u;
        while ( m_consumedCount < m_sweepCount )
        {
            // CAFFA_INFO( "Trying to pop value" << m_buffer.size() );
            auto package = m_ptrToField->pop();
            if ( package )
            {
                m_packageHandlingFunction( std::move( *package ) );
                m_consumedCount++;
                failedPops = 0u;
            }
            else
            {
                if ( failedPops > 100u )
                {
                    CAFFA_ERROR( "Failed to pop 100 in a row" );
                    return;
                }
                failedPops++;
            }
            // CAFFA_INFO( "Popped value" << m_buffer.size() );
        }
        // CAFFA_INFO( "Finished consumer" );
    }

    ConsumptionField* m_ptrToField;
    size_t            m_sweepCount;
    size_t            m_consumedCount;

    PackageHandlingFunction m_packageHandlingFunction;
};

} // End of namespace caffa
