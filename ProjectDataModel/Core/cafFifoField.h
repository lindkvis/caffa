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

namespace caffa
{
/**
 * Templated FIFO (First in, First Out) circular buffer field
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
        , m_readIndex( 0u )
        , m_writeIndex( 0u )
        , m_active( false )
    {
        m_buffer.resize( m_bufferSize, std::vector<DataType>( m_packageSize ) );
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        m_readIndex  = 0u;
        m_writeIndex = 0u;
    }

    virtual std::vector<FieldDataType> pop()                                  = 0;
    virtual void                       push( std::vector<DataType>& package ) = 0;

    virtual size_t droppedPackages() const = 0;
    size_t         packageSize() const { return m_packageSize; }

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

protected:
    bool empty() const
    {
        // CAFFA_TRACE( "Indices: " << m_writeIndex << ", " << m_readIndex );
        return m_writeIndex <= m_readIndex;
    }
    bool full() const { return m_writeIndex - m_readIndex >= m_bufferSize; }

protected:
    bool  m_active;
    Queue m_buffer;

    size_t m_bufferSize;
    size_t m_packageSize;

    size_t m_readIndex;
    size_t m_writeIndex;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;
};

template <typename DataType>
class FifoBlockingField : public FifoField<DataType>
{
public:
    using FieldDataType = DataType;

    FifoBlockingField( size_t bufferSize = 8u, size_t packageSize = 32u )
        : FifoField<DataType>( bufferSize, packageSize )
    {
    }
    std::vector<FieldDataType> pop() override
    {
        std::vector<FieldDataType> package( this->m_packageSize );

        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 100ms, [this]() { return !this->empty(); } );

        if ( !this->empty() )
        {
            package.swap( this->m_buffer[this->m_readIndex++ % this->m_bufferSize] );
        }
        lock.unlock();
        this->m_dataAccessGuard.notify_one();

        return package;
    }

    void push( std::vector<DataType>& package ) override
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 100ms, [this]() { return !this->full(); } );
        if ( !this->full() )
        {
            this->m_buffer[this->m_writeIndex++ % this->m_bufferSize].swap( package );
            lock.unlock();
            this->m_dataAccessGuard.notify_one();
        }
    }
    size_t droppedPackages() const override { return 0u; }
};

template <typename DataType>
class FifoBoundedField : public FifoField<DataType>
{
public:
    using FieldDataType = DataType;

    FifoBoundedField( size_t bufferSize = 8u, size_t packageSize = 32u )
        : FifoField<DataType>( bufferSize, packageSize )
        , m_droppedPackages( 0u )
    {
    }
    std::vector<FieldDataType> pop() override
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        this->m_dataAccessGuard.wait_for( lock, 250us, [this]() { return !this->empty(); } );
        if ( this->empty() ) return {};

        std::vector<FieldDataType> package( this->m_packageSize );
        package.swap( this->m_buffer[this->m_readIndex++ % this->m_bufferSize] );

        return package;
    }

    void push( std::vector<DataType>& package ) override
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );

        if ( this->full() )
        {
            this->m_readIndex++;
            this->m_droppedPackages++;
        }

        this->m_buffer[this->m_writeIndex++ % this->m_bufferSize].swap( package );
        lock.unlock();
        this->m_dataAccessGuard.notify_one();
    }

    size_t droppedPackages() const override
    {
        std::unique_lock<std::mutex> lock( this->m_dataAccessMutex );
        return this->m_droppedPackages;
    }

private:
    size_t m_droppedPackages;
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

template <typename DataType>
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
        size_t failedPops = 0u;
        while ( m_consumedCount < m_sweepCount )
        {
            // CAFFA_INFO( "Trying to pop value" << m_buffer.size() );
            auto package = m_ptrToField->pop();
            if ( !package.empty() )
            {
                m_packageHandlingFunction( std::move( package ) );
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
