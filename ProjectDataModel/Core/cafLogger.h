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
#include <mutex>
#include <sstream>
#include <string>

namespace caffa
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

    Level             m_currentLogLevel;
    static std::mutex s_outMutex;
};

} // namespace caffa

#define CAFFA_LOG( LogLevel_, Message_ )                                                                            \
    caffa::Logger( LogLevel_ )( static_cast<std::ostringstream&>( std::ostringstream().flush() << Message_ ).str(), \
                                __FUNCTION__,                                                                       \
                                __FILE__,                                                                           \
                                __LINE__ );

#define CAFFA_ERROR( Message_ ) CAFFA_LOG( caffa::Logger::Level::ERROR, Message_ )
#define CAFFA_WARNING( Message_ ) CAFFA_LOG( caffa::Logger::Level::WARNING, Message_ )
#define CAFFA_INFO( Message_ ) CAFFA_LOG( caffa::Logger::Level::INFO, Message_ )
#define CAFFA_DEBUG( Message_ ) CAFFA_LOG( caffa::Logger::Level::DEBUG, Message_ )
#define CAFFA_TRACE( Message_ ) CAFFA_LOG( caffa::Logger::Level::TRACE, Message_ )