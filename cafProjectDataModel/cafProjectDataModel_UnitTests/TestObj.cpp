#include "TestObj.h"

CAF_SOURCE_INIT( TestObj, "TestObj" );

TestObj::TestObj()
{
    initObject();
    initField( m_position, "Position" ).withDefault( 8765.2 );
}

TestObj::~TestObj()
{
}
