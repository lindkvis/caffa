#include "Parent.h"
#include "Child.h"

CAFFA_SOURCE_INIT( Parent );

Parent::Parent()
{
    initField( m_simpleObjectsField, "SimpleObjects" );
    initField( m_simpleObjectF, "SimpleObject" );
}

void Parent::doSome()
{
    size_t i = m_simpleObjectsField.size();
    if ( i )
    {
        // Child* c = m_simpleObjectsField[0];
        // TestObj* to = c->m_testObj();
    }
}
