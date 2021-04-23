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

#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

using namespace std::chrono_literals;

//#define DROP_PACKAGES 1

namespace caffa
{
template <typename DataType, size_t BUFFER_SIZE = 8u, size_t PACKAGE_SIZE = 32u>
class FifoField : public FieldHandle
{
public:
    using FieldDataType = DataType;
    using Queue         = std::array<std::array<FieldDataType, PACKAGE_SIZE>, BUFFER_SIZE>;

    static const size_t bufferSize  = BUFFER_SIZE;
    static const size_t packageSize = PACKAGE_SIZE;

public:
    FifoField()
        : m_droppedPackages( 0u )
        , m_readIndex( 0u )
        , m_writeIndex( 0u )
    {
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        m_readIndex  = 0u;
        m_writeIndex = 0u;
    }

    std::array<FieldDataType, PACKAGE_SIZE> pop()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );

        m_dataAccessGuard.wait( lock, [this]() { return !this->empty(); } );

        std::array<FieldDataType, PACKAGE_SIZE> package;
        package.swap( m_buffer[m_readIndex++ % BUFFER_SIZE] );

        lock.unlock();
        m_dataAccessGuard.notify_one();
        return package;
    }

    void push( std::array<DataType, PACKAGE_SIZE>& package )
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

        m_buffer[m_writeIndex++ % BUFFER_SIZE].swap( package );
        lock.unlock();
        m_dataAccessGuard.notify_one();
    }

    size_t droppedPackages() const
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        return m_droppedPackages;
    }

private:
    bool empty() const
    {
        // CAFFA_TRACE( "Indices: " << m_writeIndex << ", " << m_readIndex );
        return m_writeIndex <= m_readIndex;
    }
    bool full() const { return m_writeIndex - m_readIndex >= BUFFER_SIZE; }

private:
    Queue m_buffer;

    size_t m_droppedPackages;

    size_t m_readIndex;
    size_t m_writeIndex;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;
};

} // End of namespace caffa
