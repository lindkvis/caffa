//##################################################################################################
//
//   Caffa
//   Copyright (C) 2021- 3D-Radar AS
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

#include <iostream>
#include <sstream>
#include <string>

namespace caf
{
class Logger
{
public:
    enum class Level
    {
        NONE,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        TRACE
    };

    Logger( Level level );
    ~Logger() = default;

    Logger& operator()( const std::string& message, char const* function, char const* file, int line );

    static void setApplicationLogLevel( Level applicationLogLevel );

    static std::string logLevelPrefix( Level level );

private:
    static Level s_applicationLogLevel;

    Level m_currentLogLevel;
};

} // namespace caf

#define CAF_LOG( LogLevel_, Message_ )                                                                            \
    caf::Logger( LogLevel_ )( static_cast<std::ostringstream&>( std::ostringstream().flush() << Message_ ).str(), \
                              __FUNCTION__,                                                                       \
                              __FILE__,                                                                           \
                              __LINE__ );

#define CAF_ERROR( Message_ ) CAF_LOG( caf::Logger::Level::ERROR, Message_ )
#define CAF_WARNING( Message_ ) CAF_LOG( caf::Logger::Level::WARNING, Message_ )
#define CAF_INFO( Message_ ) CAF_LOG( caf::Logger::Level::INFO, Message_ )
#define CAF_DEBUG( Message_ ) CAF_LOG( caf::Logger::Level::DEBUG, Message_ )
#define CAF_TRACE( Message_ ) CAF_LOG( caf::Logger::Level::TRACE, Message_ )