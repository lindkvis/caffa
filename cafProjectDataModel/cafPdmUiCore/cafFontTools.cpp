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

#include <QApplication>
#include <QDesktopWidget>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::FontTools::absolutePointSize( int normalPointSize, Size relativeSize )
{
    int delta = 0;
    switch ( relativeSize )
    {
        case caf::FontTools::Size::XXSmall:
            delta = -4;
            break;
        case caf::FontTools::Size::XSmall:
            delta = -2;
            break;
        case caf::FontTools::Size::Small:
            delta = -1;
            break;
        case caf::FontTools::Size::Medium:
            delta = 0;
            break;
        case caf::FontTools::Size::Large:
            delta = +1;
            break;
        case caf::FontTools::Size::XLarge:
            delta = +2;
            break;
        case caf::FontTools::Size::XXLarge:
            delta = +4;
            break;
        default:
            break;
    }
    return normalPointSize + delta;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::FontTools::pointSizeToPixelSize( int pointSize )
{
    auto app = dynamic_cast<const QApplication*>( QCoreApplication::instance() );
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pointSize / 72.0;
        return static_cast<int>( std::ceil( inches * dpi ) );
    }
    return pointSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caf::FontTools::pixelSizeToPointSize( int pixelSize )
{
    auto app = dynamic_cast<const QApplication*>( QCoreApplication::instance() );
    if ( app )
    {
        int    dpi    = app->desktop()->logicalDpiX();
        double inches = pixelSize / dpi;
        return static_cast<int>( std::ceil( inches * 72.0 ) );
    }
    return pixelSize;
}
