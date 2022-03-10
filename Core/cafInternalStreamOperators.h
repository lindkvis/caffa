#pragma once

#include <iostream>
#include <vector>

//==================================================================================================
/// QTextStream Stream operator overloading for std::vector of things.
/// Makes automated IO of Field< std::vector< Whatever > possible as long as
/// the type will print as one single word
//==================================================================================================

template <typename T>
std::ostream& operator<<( std::ostream& str, const std::vector<T>& sobj )
{
    size_t i;
    for ( i = 0; i < sobj.size(); ++i )
    {
        str << sobj[i] << " ";
    }
    return str;
}

template <typename T>
std::istream& operator>>( std::istream& str, std::vector<T>& sobj )
{
    while ( str.good() )
    {
        T d;
        str >> d;
        if ( str.good() ) sobj.push_back( d );
    }
    return str;
}
