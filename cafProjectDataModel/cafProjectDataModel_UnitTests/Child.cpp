#include "Child.h"
#include "TestObj.h"

CAF_SOURCE_INIT( Child, "Child" );

Child::Child()
{
    CAF_InitFieldNoDefault( &m_testObj, "Numbers", "Important Numbers", "", "", "" );
}

Child::~Child()
{
}
