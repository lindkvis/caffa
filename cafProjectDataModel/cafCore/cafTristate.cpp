#include "cafTristate.h"

#include "cafStringTools.h"

#include <cctype>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Tristate::Tristate()
    : m_state( State::False )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::Tristate::operator=( const State& state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::operator==( const Tristate& other ) const
{
    return m_state == other.m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::operator==( State state ) const
{
    return m_state == state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::operator!=( const Tristate& other ) const
{
    return !( m_state == other.m_state );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::operator<( const Tristate& other ) const
{
    return m_state < other.m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Tristate::State caffa::Tristate::state() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::isTrue() const
{
    return m_state == State::True;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::isPartiallyTrue() const
{
    return m_state == State::PartiallyTrue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::Tristate::isFalse() const
{
    return m_state == State::False;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caffa::Tristate::text() const
{
    std::string txt;

    switch ( m_state )
    {
        case Tristate::State::False:
            txt = "False";
            break;
        case Tristate::State::PartiallyTrue:
            txt = "PartiallyTrue";
            break;
        case Tristate::State::True:
            txt = "True";
            break;
        default:
            break;
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::Tristate::setFromText( const std::string& valueText )
{
    std::string lowerCase = caffa::StringTools::tolower( valueText );

    if ( lowerCase == "false" )
    {
        m_state = State::False;
    }
    else if ( lowerCase == "partiallytrue" )
    {
        m_state = State::PartiallyTrue;
    }
    else if ( lowerCase == "true" )
    {
        m_state = State::True;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::istream& operator>>( std::istream& str, caffa::Tristate& triplet )
{
    std::string text;
    str >> text;
    triplet.setFromText( text );

    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::ostream& operator<<( std::ostream& str, const caffa::Tristate& triplet )
{
    str << triplet.text();

    return str;
}
