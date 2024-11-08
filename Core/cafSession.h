// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <shared_mutex>
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

    static std::shared_ptr<Session> create( Type type );

    ~Session() = default;

    const std::string& uuid() const;

    Type type() const;
    void setType( Type type );

    std::chrono::steady_clock::time_point lastKeepAlive() const;
    void                                  updateKeepAlive() const;

    static Type typeFromUint( unsigned type );

private:
    Session( Type type );

    const std::string m_uuid;
    std::atomic<Type> m_type;

    mutable std::atomic<std::chrono::steady_clock::time_point> m_lastKeepAlive;
};

} // namespace caffa