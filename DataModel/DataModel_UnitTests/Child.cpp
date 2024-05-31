#include "Child.h"
#include "TestObj.h"

CAFFA_SOURCE_INIT( Child );

Child::Child()
{
    addField( m_testObj, "Numbers" );
}

Child::~Child()
{
}
