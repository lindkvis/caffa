#include "cafStringTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caf::StringTools::trim( std::string s )
{
    s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( unsigned char ch ) { return !std::isspace( ch ); } ) );
    s.erase( std::find_if( s.rbegin(), s.rend(), []( unsigned char ch ) { return !std::isspace( ch ); } ).base(), s.end() );
    return s;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caf::StringTools::tolower( std::string data )
{
    std::transform( data.begin(), data.end(), data.begin(), []( unsigned char c ) { return (char)std::tolower( c ); } );
    return data;
}
