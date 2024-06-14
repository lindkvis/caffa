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
class SessionMaintainer;
class ConstSessionMaintainer;
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

    static std::shared_ptr<Session> create( Type                      type,
                                            std::chrono::milliseconds timeout = std::chrono::milliseconds( 1000 ) );

    ~Session() = default;

    const std::string& uuid() const;

    Type type() const;
    void setType( Type type );

    bool isExpired() const;
    void updateKeepAlive() const;

    static Type typeFromUint( unsigned type );

    std::chrono::milliseconds timeout() const;

private:
    friend class SessionMaintainer;
    friend class ConstSessionMaintainer;

    Session( Type type, std::chrono::milliseconds timeout );

    bool unlockedIsExpired() const;
    void blockExpiration() const;
    void unblockExpiration() const;

    const std::string               m_uuid;
    std::atomic<Type>               m_type;
    const std::chrono::milliseconds m_timeOut;

    mutable std::atomic<std::chrono::steady_clock::time_point> m_lastKeepAlive;
    mutable std::shared_mutex                                  m_expirationBlockedMutex;
    mutable bool                                               m_expirationBlocked;
};

class SessionMaintainer
{
public:
    SessionMaintainer( std::shared_ptr<Session> session = nullptr );
    ~SessionMaintainer();

    std::shared_ptr<Session> operator->();
    std::shared_ptr<Session> operator*();
    operator bool() const;
    bool     operator!() const;
    Session* get();

private:
    std::shared_ptr<Session> m_session;
};

class ConstSessionMaintainer
{
public:
    ConstSessionMaintainer( std::shared_ptr<const Session> session = nullptr );
    ~ConstSessionMaintainer();

    std::shared_ptr<const Session> operator->() const;
    std::shared_ptr<const Session> operator*() const;
    operator bool() const;
    bool           operator!() const;
    const Session* get() const;

private:
    std::shared_ptr<const Session> m_session;
};

} // namespace caffa