//##################################################################################################
//
//   Caffa
//   Copyright (C) 2022- Kontur AS
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
#pragma once

#include <chrono>
#include <mutex>
#include <string>

namespace caffa
{
/**
 * @brief Abstract class representing an application session
 *
 */
class Session
{
public:
    enum class Type
    {
        INVALID   = 0x0,
        REGULAR   = 0x1,
        OBSERVING = 0x2
    };

    Session( Type type, std::chrono::milliseconds timeout = std::chrono::milliseconds( 500 ) );
    virtual ~Session() = default;

    const std::string& uuid() const;

    Type type() const;

    bool isExpired() const;
    void updateKeepAlive();

    static Type typeFromUint( unsigned type );

private:
    std::string m_uuid;
    Type        m_type;

    std::chrono::steady_clock::time_point m_lastKeepAlive;
    std::chrono::milliseconds             m_timeOut;
    mutable std::mutex                    m_mutex;
};
} // namespace caffa