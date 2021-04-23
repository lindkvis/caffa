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

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

//#define DROP_PACKAGES 1

namespace caffa
{
/**
 * Templated FIFO (First in, First Out circular buffer field
 */
template <typename DataType>
class FifoField : public FieldHandle
{
public:
    using FieldDataType = DataType;
    using Queue         = std::vector<std::vector<FieldDataType>>;

public:
    FifoField( size_t bufferSize = 8u, size_t packageSize = 32u )
        : m_bufferSize( bufferSize )
        , m_packageSize( packageSize )
        , m_droppedPackages( 0u )
        , m_readIndex( 0u )
        , m_writeIndex( 0u )
    {
        m_buffer.resize( m_bufferSize, std::vector<DataType>( m_packageSize ) );
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        m_readIndex  = 0u;
        m_writeIndex = 0u;
    }

    std::vector<FieldDataType> pop()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );

        m_dataAccessGuard.wait( lock, [this]() { return !this->empty(); } );

        std::vector<FieldDataType> package;
        package.swap( m_buffer[m_readIndex++ % m_bufferSize] );

        lock.unlock();
        m_dataAccessGuard.notify_one();
        return package;
    }

    void push( std::vector<DataType>& package )
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );

#ifdef DROP_PACKAGES
        // Wait 250 nanoseconds to see if buffer has cleared
        // If not drop package
        m_dataAccessGuard.wait_for( lock, 250ns, [this]() { return !this->full(); } );
        if ( full() )
        {
            m_readIndex += BUFFER_SIZE / 2;
            m_droppedPackages += BUFFER_SIZE / 2;
        }
#else
        m_dataAccessGuard.wait( lock, [this]() { return !this->full(); } );
#endif

        m_buffer[m_writeIndex++ % m_bufferSize].swap( package );
        lock.unlock();
        m_dataAccessGuard.notify_one();
    }

    size_t droppedPackages() const
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        return m_droppedPackages;
    }

    size_t packageSize() const { return m_packageSize; }

private:
    bool empty() const
    {
        // CAFFA_TRACE( "Indices: " << m_writeIndex << ", " << m_readIndex );
        return m_writeIndex <= m_readIndex;
    }
    bool full() const { return m_writeIndex - m_readIndex >= m_bufferSize; }

private:
    Queue m_buffer;

    size_t m_bufferSize;
    size_t m_packageSize;
    size_t m_droppedPackages;

    size_t m_readIndex;
    size_t m_writeIndex;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;
};

template <typename DataType>
class FifoProducer
{
public:
    using ProductionField = FifoField<DataType>;
    using Package         = std::vector<DataType>;

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
    }
    void produce()
    {
        while ( validProductionCount() < m_sweepCount )
        {
            Package package = m_packageCreator( m_doneCount++ );
            // CAFFA_INFO( "Pushed value number: " << m_doneCount - m_ptrToField->droppedPackages() << " out of "
            //                                  << m_sweepCount );

            m_ptrToField->push( package );
        }
        CAFFA_INFO( "Finished producing" );
    }
    size_t validProductionCount() const { return m_doneCount - m_ptrToField->droppedPackages(); }

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

    ProductionField*       m_ptrToField;
    bool                   m_finished;
    std::mutex             m_mutex;
    size_t                 m_doneCount;
    size_t                 m_sweepCount;
    PackageCreatorFunction m_packageCreator;
};

template <typename DataType, size_t BUFFER_SIZE = 8u, size_t PACKAGE_SIZE = 32u>
class FifoConsumer
{
public:
    using ConsumptionField = FifoField<DataType>;
    using Package          = std::vector<DataType>;

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
        while ( m_consumedCount++ < m_sweepCount )
        {
            // CAFFA_INFO( "Trying to pop value" << m_buffer.size() );
            auto package = m_ptrToField->pop();
            // CAFFA_INFO( "Popped value" << m_buffer.size() );
            m_packageHandlingFunction( std::move( package ) );
        }
        // CAFFA_INFO( "Finished consumer" );
    }

    ConsumptionField* m_ptrToField;
    size_t            m_sweepCount;
    size_t            m_consumedCount;

    PackageHandlingFunction m_packageHandlingFunction;
};

} // End of namespace caffa
