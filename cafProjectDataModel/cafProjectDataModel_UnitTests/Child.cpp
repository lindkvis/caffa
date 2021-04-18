#include "Child.h"
#include "TestObj.h"

CAF_SOURCE_INIT( Child, "Child", "Object" );

Child::Child()
{
    initField( m_testObj, "Numbers" );
}

Child::~Child()
{
}
