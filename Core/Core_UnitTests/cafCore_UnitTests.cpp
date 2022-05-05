//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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

#include "gtest.h"

#include "cafLogger.h"

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace po = boost::program_options;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    auto logLevel = caffa::Logger::Level::WARNING;

    auto        logLevels = caffa::Logger::logLevels();
    std::string logLevelString;
    for ( auto [enumValue, name] : logLevels )
    {
        if ( !logLevelString.empty() ) logLevelString += ", ";
        logLevelString += name;
    }
    std::string verbosityHelpString = "Set verbosity level (" + logLevelString + ")";

    int result = EXIT_FAILURE;

    try
    {
        po::options_description flags( "Flags" );
        flags.add_options()( "help,h", "Show help message" );
        flags.add_options()( "verbosity,V", po::value<std::string>()->value_name( "level" ), verbosityHelpString.c_str() );
        flags.add_options()( "logfile,l", po::value<std::string>()->value_name( "filename" ), "Log to provided file" );

        po::variables_map vm;
        po::store( po::parse_command_line( argc, argv, flags ), vm );
        po::notify( vm );

        if ( vm.count( "help" ) )
        {
            std::cout << flags << std::endl;
            return 1;
        }

        if ( vm.count( "logfile" ) )
        {
            caffa::Logger::setLogFile( vm["logfile"].as<std::string>() );
        }

        if ( vm.count( "verbosity" ) )
        {
            logLevel = caffa::Logger::logLevelFromLabel( vm["verbosity"].as<std::string>() );
        }
        caffa::Logger::setApplicationLogLevel( logLevel );
        caffa::Logger::registerThreadName( "main" );

        testing::InitGoogleTest( &argc, argv );
        result = RUN_ALL_TESTS();
    }
    catch ( const std::exception& e )
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 2;
    }

    return result;
}
