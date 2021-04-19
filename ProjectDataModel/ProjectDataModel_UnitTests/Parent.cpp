#include "Parent.h"
#include "Child.h"

#include "gtest.h"

CAF_SOURCE_INIT( Parent, "Parent", "Object" );

Parent::Parent()
{
    initField( m_simpleObjectsField, "SimpleObjects" );
    initField( m_simpleObjectF, "SimpleObject" );
}

Parent::~Parent()
{
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

TEST( IncludeTest, Basic )
{
    Parent* p = new Parent;
    delete ( p );
}
