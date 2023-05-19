#include "Child.h"
#include "TestObj.h"

CAFFA_SOURCE_INIT( Child );

Child::Child()
{
    initField( m_testObj, "Numbers" );
}

Child::~Child()
{
}
