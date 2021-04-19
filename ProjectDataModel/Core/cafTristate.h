#pragma once

#include <iostream>

namespace caffa
{
//==================================================================================================
//==================================================================================================
class Tristate
{
public:
    enum class State
    {
        False,
        PartiallyTrue,
        True
    };

public:
    Tristate();

    void operator=( const State& state );

    bool operator==( const Tristate& other ) const;
    bool operator==( State state ) const;
    bool operator!=( const Tristate& other ) const;
    bool operator<( const Tristate& other ) const;

    State state() const;

    bool isTrue() const;
    bool isPartiallyTrue() const;
    bool isFalse() const;

    std::string text() const;
    void        setFromText( const std::string& valueText );

protected:
    State m_state;
};

} // end namespace caffa

//==================================================================================================
// Overload of QTextStream for caffa::Tristate
//==================================================================================================
std::istream& operator>>( std::istream& str, caffa::Tristate& triplet );
std::ostream& operator<<( std::ostream& str, const caffa::Tristate& triplet );
