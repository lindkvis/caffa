#include "TestObj.h"

CAF_SOURCE_INIT( TestObj, "TestObj" );

TestObj::TestObj()
{
    initObject( "TestObj", "", "", "" );
    CAF_InitField( &m_position, "Position", 8765.2, "Position", "", "", "" );
}

TestObj::~TestObj()
{
}
