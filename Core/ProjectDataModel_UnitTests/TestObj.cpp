#include "TestObj.h"

CAFFA_SOURCE_INIT( TestObj )

TestObj::TestObj()
{
    initField( m_position, "Position" ).withDefault( 8765.2 );
}

TestObj::~TestObj()
{
}
