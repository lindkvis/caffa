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
    Session( std::chrono::milliseconds timeout = std::chrono::milliseconds( 500 ) );
    virtual ~Session() = default;

    const std::string& uuid() const;

    bool isExpired() const;
    void updateKeepAlive();

private:
    std::string m_uuid;

    std::chrono::steady_clock::time_point m_lastKeepAlive;
    std::chrono::milliseconds             m_timeOut;
};
} // namespace caffa