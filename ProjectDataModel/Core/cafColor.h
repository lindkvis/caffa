//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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

#include <array>
#include <string>
#include <tuple>

namespace caffa
{
/**
 * Simple 32-bit color class
 * This should never contain a lot of features since it is mainly to avoid depending
 * on fancy GUI-classes.
 */
class Color
{
public:
    typedef unsigned char uchar;

    enum DIM
    {
        RED = 0,
        GREEN,
        BLUE,
        ALPHA
    };
    explicit Color();
    explicit Color( uchar r, uchar g, uchar b, uchar a = 255u );
    explicit Color( int r, int g, int b, int a = 255 );
    explicit Color( float r, float g, float b, float a = 1.0 );
    explicit Color( double r, double g, double b, double a = 1.0 );
    Color( const std::string& hexColor );
    ~Color() = default;

    template <DIM dim>
    uchar get() const
    {
        return m_rgba[(int)dim];
    }

    template <DIM dim>
    float getf() const
    {
        return (float)m_rgba[(int)dim] / 255.0f;
    }

    template <DIM dim>
    void set( uchar value )
    {
        m_rgba[(int)dim] = value;
    }

    template <DIM dim>
    void setf( float value )
    {
        m_rgba[(int)dim] = (uchar)value * 255;
    }

    std::tuple<uchar, uchar, uchar>        rgb() const;
    std::tuple<uchar, uchar, uchar, uchar> rgba() const;

    template <class ColorT>
    ColorT to() const
    {
        return ColorT( m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3] );
    }

    bool        operator==( const Color& rhs ) const;
    bool        operator<( const Color& rhs ) const;
    std::string hexString() const;

private:
    static std::array<uchar, 4> fromHexString( const std::string& hexString );

private:
    std::array<uchar, 4> m_rgba;
};

} // namespace caffa

//==================================================================================================
/// Implementation of stream operators to make Colors work smoothly
//==================================================================================================

std::istream& operator>>( std::istream& str, caffa::Color& color );

std::ostream& operator<<( std::ostream& str, const caffa::Color& color );
