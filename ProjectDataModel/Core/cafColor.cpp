#include "cafColor.h"

#include "cafAssert.h"

#include <iomanip>
#include <sstream>
#include <vector>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color( uchar r, uchar g, uchar b, uchar a /*= 255u */ )
    : m_rgba( { r, g, b, a } )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color( int r, int g, int b, int a /*= 255 */ )
    : Color( (uchar)r, (uchar)g, (uchar)b, (uchar)a )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color( float rf, float gf, float bf, float af /*= 1.0 */ )
    : m_rgba( { static_cast<uchar>( rf * 255.0 ),
                static_cast<uchar>( gf * 255.0 ),
                static_cast<uchar>( bf * 255.0 ),
                static_cast<uchar>( af * 255.0 ) } )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color( double r, double g, double b, double a /*= 1.0 */ )
    : Color( (float)r, (float)g, (float)b, (float)a )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color( const std::string& hexColor )
    : m_rgba( fromHexString( hexColor ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color::Color()
    : m_rgba( { 0, 0, 0, 0 } )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<Color::uchar, Color::uchar, Color::uchar> Color::rgb() const
{
    return std::make_tuple( m_rgba[0], m_rgba[1], m_rgba[2] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<Color::uchar, Color::uchar, Color::uchar, Color::uchar> Color::rgba() const
{
    return std::make_tuple( m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<Color::uchar, 4> Color::fromHexString( const std::string& hexString )
{
    size_t stringLength = hexString.length();
    CAFFA_ASSERT( ( stringLength == 7u || stringLength == 9u ) && hexString[0] == '#' );

    uchar red   = (uchar)strtol( hexString.substr( 1, 2 ).c_str(), nullptr, 16 );
    uchar green = (uchar)strtol( hexString.substr( 3, 2 ).c_str(), nullptr, 16 );
    uchar blue  = (uchar)strtol( hexString.substr( 5, 2 ).c_str(), nullptr, 16 );
    uchar alpha = stringLength == 9u ? (uchar)strtol( hexString.substr( 7, 2 ).c_str(), nullptr, 16 ) : 255u;
    return { red, green, blue, alpha };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Color::operator==( const Color& rhs ) const
{
    return m_rgba == rhs.m_rgba;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Color::operator<( const Color& rhs ) const
{
    return m_rgba < rhs.m_rgba;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caffa::Color::hexString() const
{
    auto [red, green, blue, alpha] = rgba();

    std::stringstream ss;
    ss << "#" << std::hex << std::setfill( '0' ) << red;
    ss << std::setfill( '0' ) << green;
    ss << std::setfill( '0' ) << blue;
    if ( alpha != 255 )
    {
        ss << std::setfill( '0' ) << alpha;
    }
    return ss.str();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::istream& operator>>( std::istream& str, caffa::Color& color )
{
    std::string text;
    str >> text;
    color = caffa::Color( text );
    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& str, const caffa::Color& color )
{
    std::string text = color.hexString();
    str << text;
    return str;
}
