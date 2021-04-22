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

using namespace std::chrono_literals;

namespace caffa
{
class FifoFieldHandle : public FieldHandle
{
public:
    virtual size_t packageSize() const   = 0;
    virtual void   clear()               = 0;
    virtual size_t droppedValues() const = 0;
};

template <typename DataType, size_t MAX_BUFFER_SIZE = 8u>
class FifoField : public FifoFieldHandle
{
public:
    using FieldDataType = DataType;
    using Queue         = std::queue<std::vector<FieldDataType>>;

public:
    FifoField( size_t packageSize = 32u )
        : m_packageSize( packageSize )
        , m_droppedValues( 0u )
    {
    }

    void clear() override
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        Queue().swap( m_buffer );
    }

    std::vector<FieldDataType> pop()
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );

        m_dataAccessGuard.wait( lock, [this]() { return !m_buffer.empty(); } );

        std::vector<FieldDataType> package = m_buffer.front();
        m_buffer.pop();
        lock.unlock();
        m_dataAccessGuard.notify_one();
        return package;
    }

    void push( const std::vector<DataType>& data )
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );

        m_dataAccessGuard.wait( lock, [this]() { return m_buffer.size() < MAX_BUFFER_SIZE; } );
        // Clear item from buffer if we've reached max size
        /* if ( m_buffer.size() >= MAX_BUFFER_SIZE )
        {
            //            lock.unlock();
            //          m_dataAccessGuard.notify_one();
            //        lock.lock();
            //      m_dataAccessGuard.wait_for( lock, 1ns, [this]() { return m_buffer.size() < MAX_BUFFER_SIZE; } );
            //    if ( m_buffer.size() >= MAX_BUFFER_SIZE )
            {
                // CAFFA_WARNING( "Dropping a package!!" );
                auto oldData = m_buffer.front();
                m_buffer.pop();
                m_droppedValues += oldData.size();
            }
            // else
            //{
            // CAFFA_INFO( "Salvaged dropping a package" );
            //}
        } */

        m_buffer.push( data );
        lock.unlock();
        m_dataAccessGuard.notify_one();
    }

    size_t packageSize() const override { return m_packageSize; }
    size_t droppedValues() const override
    {
        std::unique_lock<std::mutex> lock( m_dataAccessMutex );
        return m_droppedValues;
    }

private:
    Queue m_buffer;

    size_t m_packageSize;
    size_t m_droppedValues;

    mutable std::mutex      m_dataAccessMutex;
    std::condition_variable m_dataAccessGuard;
};

} // End of namespace caffa
