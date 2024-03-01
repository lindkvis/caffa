// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
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

#include "cafStringEncoding.h"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace caffa::StringTools
{

std::string decodeBase64( std::string input )
{
    using namespace boost::archive::iterators;
    typedef transform_width<binary_from_base64<remove_whitespace<std::string::const_iterator>>, 8, 6> ItBinaryT;

    try
    {
        // If the input isn't a multiple of 4, pad with =
        size_t num_pad_chars( ( 4 - input.size() % 4 ) % 4 );
        input.append( num_pad_chars, '=' );

        size_t pad_chars( std::count( input.begin(), input.end(), '=' ) );
        std::replace( input.begin(), input.end(), '=', 'A' );
        std::string output( ItBinaryT( input.begin() ), ItBinaryT( input.end() ) );
        output.erase( output.end() - pad_chars, output.end() );
        return output;
    }
    catch ( const std::exception& )
    {
        return std::string( "" );
    }
}

std::string encodeBase64( std::string val )
{
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;

    if ( val.empty() ) return val;

    auto tmp = std::string( It( std::begin( val ) ), It( std::end( val ) ) );
    return tmp.append( ( 3 - val.size() % 3 ) % 3, '=' );
}
} // namespace caffa::StringTools