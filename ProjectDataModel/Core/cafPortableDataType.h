//##################################################################################################
//
//   Caffa
//   Copyright (C) 3D-Radar AS
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

#pragma once

#include <string>
#include <vector>

#define CONCAT( a, b ) a b

namespace caffa
{
class ObjectHandle;

/**
 * The default non-portable mangled type id
 */
template <typename DataType>
struct PortableDataType
{
    constexpr const char* name() { return typeid( DataType ).name(); }
};

/**
 * Specialisations for common data types
 */
#define CAFFA_DEFINE_PORTABLE_TYPE( DataType )                                    \
    template <>                                                                   \
    struct PortableDataType<DataType>                                             \
    {                                                                             \
        static constexpr const char* name() { return #DataType; }                 \
    };                                                                            \
                                                                                  \
    template <>                                                                   \
    struct PortableDataType<std::vector<DataType>>                                \
    {                                                                             \
        static constexpr const char* name() { return CONCAT( #DataType, "[]" ); } \
    };

#define CAFFA_DEFINE_PORTABLE_TYPE_NAME( DataType, StringAlias )                    \
    template <>                                                                     \
    struct PortableDataType<DataType>                                               \
    {                                                                               \
        static constexpr const char* name() { return StringAlias; }                 \
    };                                                                              \
                                                                                    \
    template <>                                                                     \
    struct PortableDataType<std::vector<DataType>>                                  \
    {                                                                               \
        static constexpr const char* name() { return CONCAT( StringAlias, "[]" ); } \
    };

CAFFA_DEFINE_PORTABLE_TYPE( int )
CAFFA_DEFINE_PORTABLE_TYPE( unsigned )
CAFFA_DEFINE_PORTABLE_TYPE( double )
CAFFA_DEFINE_PORTABLE_TYPE( float )
CAFFA_DEFINE_PORTABLE_TYPE( bool )
CAFFA_DEFINE_PORTABLE_TYPE( char )
CAFFA_DEFINE_PORTABLE_TYPE_NAME( uint64_t, "uint64" )
CAFFA_DEFINE_PORTABLE_TYPE_NAME( int64_t, "int64" )
CAFFA_DEFINE_PORTABLE_TYPE_NAME( std::string, "string" )
}; // namespace caffa