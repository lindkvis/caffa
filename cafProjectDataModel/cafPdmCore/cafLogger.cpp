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

using namespace caf;

Logger::Level Logger::s_applicationLogLevel = Logger::Level::WARNING;

Logger::Logger( Level level )
    : m_currentLogLevel( level )
{
}

Logger& Logger::operator()( const std::string& message, char const* function, char const* file, int line )
{
    if ( m_currentLogLevel <= s_applicationLogLevel )
    {
        // TODO: should provide platform specific path delimiter
        auto filePath = caf::StringTools::split( file, "/" );
        auto fileName = !filePath.empty() ? filePath.back() : file;
        std::cout << logLevelPrefix( m_currentLogLevel ) << fileName << "::" << function << "()[" << line
                  << "]: " << message << std::endl;
    }
    return *this;
}

void Logger::setApplicationLogLevel( Level applicationLogLevel )
{
    s_applicationLogLevel = applicationLogLevel;
}

std::string Logger::logLevelPrefix( Level level )
{
    switch ( level )
    {
        case Level::ERROR:
            return "ERROR: ";
        case Level::WARNING:
            return "WARNING: ";
        case Level::INFO:
            return "INFO: ";
        case Level::DEBUG:
            return "DEBUG: ";
        case Level::TRACE:
            return "TRACE: ";
        default:
            return "";
    }
    return "";
}