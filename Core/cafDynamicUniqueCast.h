//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2021-2022 3D-Radar AS
//   Copyright (C) 2022-     Kontur AS
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

#include "cafAssert.h"
#include "cafLogger.h"

#include <memory>

namespace caffa
{
template <typename ToClass, typename FromClass>
bool dynamic_unique_cast_is_valid( const std::unique_ptr<FromClass>& fromPointer )
{
    return dynamic_cast<ToClass*>( fromPointer.get() ) != nullptr;
}

template <typename ToClass, typename FromClass>
std::unique_ptr<ToClass> dynamic_unique_cast( std::unique_ptr<FromClass> fromPointer )
{
    if ( !dynamic_unique_cast_is_valid<ToClass, FromClass>( fromPointer ) )
    {
        CAFFA_ERROR( "Bad cast! " << typeid( FromClass ).name() << " cannot be cast to " << typeid( ToClass ).name() );
        CAFFA_ASSERT( false );
        return nullptr;
    }

    std::unique_ptr<ToClass> toPointer( static_cast<ToClass*>( fromPointer.release() ) );
    return toPointer;
}
}