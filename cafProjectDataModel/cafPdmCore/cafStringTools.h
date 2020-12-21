/////////////////////////////////////////////////////////////////////////////////////////////////
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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
/////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cctype>
#include <functional>
#include <locale>
#include <numeric>
#include <regex>
#include <string>

namespace caf::StringTools
{
template <class InputIt>
std::string join( InputIt first, InputIt last, const std::string& delimiter )
{
    if ( last == first ) return std::string();
    return std::accumulate( next( first ), // there is at least 1 element, so OK.
                            last,
                            *first, // the initial value
                            [&delimiter]( auto a, auto b ) { return a + delimiter + b; } );
}

template <class Container = std::list<std::string>>
Container split( const std::string& string, const std::string& delimiter )
{
    static_assert( std::is_same<typename Container::value_type, std::string>::value,
                   "split() only creates containers of std::strings" );
    Container output;

    size_t start = 0u;
    size_t end   = string.find( delimiter );
    while ( end != std::string::npos )
    {
        output.push_back( string.substr( start, end - start ) );
        start = end + delimiter.length();
        end   = string.find( delimiter, start );
    }
    output.push_back( string.substr( start, end ) );
    return output;
}

template <class Container = std::list<std::string>>
Container split( const std::string& string, const std::regex& regex, bool skipEmptyParts = false )
{
    Container output;

    std::sregex_token_iterator it( string.begin(), string.end(), regex, -1 );
    std::sregex_token_iterator end;

    while ( it != end )
    {
        auto token = *it++;
        if ( !skipEmptyParts || token.length() > 0u ) output.push_back( token );
    }

    return output;
}

std::string trim( std::string s );
std::string tolower( std::string data );

} // namespace caf::StringTools
