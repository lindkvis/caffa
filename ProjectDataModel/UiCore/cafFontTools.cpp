//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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
#include "cafFontTools.h"

#include "cafAppEnum.h"

#include <cmath>

using namespace caffa;

template <>
void FontTools::FontSizeEnum::setUp()
{
    addItem( FontTools::FontSize::FONT_SIZE_8, "8", "8" );
    addItem( FontTools::FontSize::FONT_SIZE_10, "10", "10" );
    addItem( FontTools::FontSize::FONT_SIZE_12, "12", "12" );
    addItem( FontTools::FontSize::FONT_SIZE_14, "14", "14" );
    addItem( FontTools::FontSize::FONT_SIZE_16, "16", "16" );
    addItem( FontTools::FontSize::FONT_SIZE_24, "24", "24" );
    addItem( FontTools::FontSize::FONT_SIZE_32, "32", "32" );

    setDefault( FontTools::FontSize::FONT_SIZE_8 );
}

template <>
void FontTools::DeltaSizeEnum::setUp()
{
    addItem( FontTools::DeltaSize::XSmall, "XX Small", "XX Small" );
    addItem( FontTools::DeltaSize::XSmall, "X Small", "X Small" );
    addItem( FontTools::DeltaSize::Small, "Small", "Small" );
    addItem( FontTools::DeltaSize::Medium, "Medium", "Medium" );
    addItem( FontTools::DeltaSize::Large, "Large", "Medium" );
    addItem( FontTools::DeltaSize::XLarge, "X Large", "X Large" );
    addItem( FontTools::DeltaSize::XXLarge, "XX Large", "XX Large" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::absolutePointSize( FontSize normalPointSize, DeltaSize relativeSize )
{
    return static_cast<int>( normalPointSize ) + static_cast<int>( relativeSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pointSizeToPixelSize( int pointSize, int dpi )
{
    double inches = pointSize / 72.0;
    return static_cast<int>( std::ceil( inches * dpi ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pointSizeToPixelSize( FontSize pointSize, int dpi )
{
    return pointSizeToPixelSize( (int)pointSize, dpi );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FontTools::pixelSizeToPointSize( int pixelSize, int dpi )
{
    double inches = pixelSize / dpi;
    return static_cast<int>( std::ceil( inches * 72.0 ) );
}
