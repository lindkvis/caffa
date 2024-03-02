// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2022- Kontur AS
//
//    This library may be used under the terms of the GNU Lesser General Public License as follows:
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
// ##################################################################################################
#include "cafUuidGenerator.h"

#include "cafLogger.h"

#include "uuid.h"

#include <chrono>
#include <regex>

using namespace caffa;
using namespace std::chrono;

std::unique_ptr<uuids::uuid_random_generator> UuidGenerator::s_uuidGenerator;
std::mutex                                    UuidGenerator::s_mutex;

std::string UuidGenerator::generate()
{
    std::scoped_lock lock( s_mutex );
    if ( !s_uuidGenerator )
    {
        auto     now                           = steady_clock::now();
        auto     nanoseconds_since_epoch       = now.time_since_epoch();
        auto     seconds_since_epoch           = duration_cast<seconds>( nanoseconds_since_epoch );
        unsigned nanoseconds_since_last_second = static_cast<unsigned>(
            ( nanoseconds_since_epoch - duration_cast<nanoseconds>( seconds_since_epoch ) ).count() );

        s_uuidGenerator =
            std::make_unique<uuids::uuid_random_generator>( new std::mt19937( nanoseconds_since_last_second ) );
    }

    return uuids::to_string( ( *s_uuidGenerator )() );
}

bool UuidGenerator::isUuid( const std::string& string )
{
    static const std::regex e( "^[0-9a-f]{8}-[0-9a-f]{4}-[0-5][0-9a-f]{3}-[089ab][0-9a-f]{3}-[0-9a-f]{12}$",
                               std::regex_constants::icase );
    return std::regex_match( string, e );
}