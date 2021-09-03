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
#include "cafLogger.h"

#include "cafStringTools.h"

#include <fstream>
#include <thread>

using namespace caffa;

Logger::Level                 Logger::s_applicationLogLevel = Logger::Level::WARNING;
std::unique_ptr<std::ostream> Logger::s_stream              = std::make_unique<std::ostream>( std::cout.rdbuf() );

void Logger::log( Level level, const std::string& message, char const* function, char const* file, int line )
{
    if ( level <= s_applicationLogLevel )
    {
        // TODO: should provide platform specific path delimiter
        auto filePath = caffa::StringTools::split( file, "/" );
        auto fileName = !filePath.empty() ? filePath.back() : file;
        *s_stream << logLevelLabel( level ) << ": " << fileName << "::" << function << "()[" << line << "]: " << message
                  << std::endl;
    }
}

Logger::Level Logger::applicationLogLevel()
{
    return s_applicationLogLevel;
}
void Logger::setApplicationLogLevel( Level applicationLogLevel )
{
    s_applicationLogLevel = applicationLogLevel;
}

std::string Logger::logLevelLabel( Level level )
{
    return Logger::logLevels()[level];
}

Logger::Level Logger::logLevelFromLabel( const std::string& label )
{
    for ( int i = (int)Level::OFF; i <= (int)Level::TRACE; ++i )
    {
        Level level = (Level)i;
        if ( logLevelLabel( level ) == label )
        {
            return level;
        }
    }
    return Logger::Level::OFF;
}

void Logger::setLogFile( const std::string& logFile )
{
    s_stream = std::make_unique<std::ofstream>( logFile );
}

std::map<Logger::Level, std::string> Logger::logLevels()
{
    return { { Level::OFF, "off" },
             { Level::TRACE, "trace" },
             { Level::DEBUG, "debug" },
             { Level::INFO, "info" },
             { Level::WARNING, "warning" },
             { Level::ERROR, "error" },
             { Level::CRITICAL, "critical" } };
}
