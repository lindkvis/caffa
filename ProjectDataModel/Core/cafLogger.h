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
#include <map>
#include <memory>
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
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        OFF
    };

    static void        log( Level level, const std::string& message, char const* function, char const* file, int line );
    static Level       applicationLogLevel();
    static void        setApplicationLogLevel( Level applicationLogLevel );
    static std::string logLevelLabel( Level level );
    static Level       logLevelFromLabel( const std::string& label );
    static void        setLogFile( const std::string& logFile );
    static std::map<Level, std::string> logLevels();

private:
    static Level                         s_applicationLogLevel;
    static std::unique_ptr<std::ostream> s_stream;
};

} // namespace caffa

#define CAFFA_LOG( LOG_LEVEL, MESSAGE )                                                                    \
    caffa::Logger::log( LOG_LEVEL,                                                                         \
                        static_cast<std::ostringstream&>( std::ostringstream().flush() << MESSAGE ).str(), \
                        __FUNCTION__,                                                                      \
                        __FILE__,                                                                          \
                        __LINE__ );

#define CAFFA_CRITICAL( Message_ )                             \
    {                                                          \
        CAFFA_LOG( caffa::Logger::Level::CRITICAL, Message_ ); \
        exit( 1 );                                             \
    }
#define CAFFA_ERROR( Message_ ) CAFFA_LOG( caffa::Logger::Level::ERROR, Message_ )
#define CAFFA_WARNING( Message_ ) CAFFA_LOG( caffa::Logger::Level::WARNING, Message_ )
#define CAFFA_INFO( Message_ ) CAFFA_LOG( caffa::Logger::Level::INFO, Message_ )
#define CAFFA_DEBUG( Message_ ) CAFFA_LOG( caffa::Logger::Level::DEBUG, Message_ )
#define CAFFA_TRACE( Message_ ) CAFFA_LOG( caffa::Logger::Level::TRACE, Message_ )
