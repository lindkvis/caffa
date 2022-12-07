
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObject.h"

#include <string>

class MsjSimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    MsjSimpleObj()
        : Object()
    {
        initField( name, "Name" );
        initField( id, "ID" );

        static int a = 0;

        id   = a++;
        name = std::string( "Name" ) + std::to_string( id );
    }

    caffa::Field<std::string> name;
    caffa::Field<int>         id;
};

CAFFA_SOURCE_INIT( MsjSimpleObj, "MsjSimpleObj", "Object" );

class SimpleObjDerived : public MsjSimpleObj
{
    CAFFA_HEADER_INIT;

public:
    SimpleObjDerived()
        : MsjSimpleObj()
    {
        initField( valueA, "valueA" );
    }

    caffa::Field<int> valueA;
};

CAFFA_SOURCE_INIT( SimpleObjDerived, "SimpleObjDerived", "MsjSimpleObj", "Object" );

class SimpleObjDerivedOther : public MsjSimpleObj
{
    CAFFA_HEADER_INIT;

public:
    SimpleObjDerivedOther()
        : MsjSimpleObj()
    {
        initField( valueDouble, "valueDouble" );
    }

    caffa::Field<double> valueDouble;
};

CAFFA_SOURCE_INIT( SimpleObjDerivedOther, "SimpleObjDerivedOther", "MsjSimpleObj", "Object" );

class ContainerObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    ContainerObj()
        : Object()
    {
        initField( derivedObjs, "derivedObjs" );
        initField( derivedOtherObjs, "derivedOtherObjs" );
    }

    ~ContainerObj() {}

    caffa::ChildArrayField<SimpleObjDerived*>      derivedObjs;
    caffa::ChildArrayField<SimpleObjDerivedOther*> derivedOtherObjs;
};

CAFFA_SOURCE_INIT( ContainerObj, "ContainerObj", "Object" );

template <class U, typename T>
U findObjectById( T start, T end, int id )
{
    for ( T it = start; it != end; it++ )
    {
        if ( id == ( *it )->id.value() )
        {
            return *it;
        }
    }

    return nullptr;
}

TEST( ChildArrayFieldHandle, DerivedObjects )
{
    auto containerObj = std::make_unique<ContainerObj>();

    auto s0 = std::make_unique<SimpleObjDerived>();
    auto s1 = std::make_unique<SimpleObjDerived>();
    auto s2 = std::make_unique<SimpleObjDerived>();

    containerObj->derivedObjs.push_back( std::move( s0 ) );
    containerObj->derivedObjs.push_back( std::move( s1 ) );
    auto s2p = s2.get();
    containerObj->derivedObjs.push_back( std::move( s2 ) );

    auto allObjects = containerObj->derivedObjs.objects();

    SimpleObjDerived* myObj = findObjectById<SimpleObjDerived*>( allObjects.begin(), allObjects.end(), 2 );
    EXPECT_EQ( s2p, myObj );

    myObj = findObjectById<SimpleObjDerived*>( allObjects.begin(), allObjects.end(), -1 );
    EXPECT_EQ( nullptr, myObj );
}

TEST( ChildArrayFieldHandle, DerivedOtherObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    auto s0 = std::make_unique<SimpleObjDerivedOther>();
    auto s1 = std::make_unique<SimpleObjDerivedOther>();
    auto s2 = std::make_unique<SimpleObjDerivedOther>();

    int s2Id = s2->id;

    containerObj->derivedOtherObjs.push_back( std::move( s0 ) );
    containerObj->derivedOtherObjs.push_back( std::move( s1 ) );
    auto s2p = s2.get();
    containerObj->derivedOtherObjs.push_back( std::move( s2 ) );

    auto allObjects = containerObj->derivedOtherObjs.objects();

    SimpleObjDerivedOther* myObj = findObjectById<SimpleObjDerivedOther*>( allObjects.begin(), allObjects.end(), s2Id );

    EXPECT_EQ( s2p, myObj );

    containerObj->derivedOtherObjs.removeChildObject( myObj );

    allObjects = containerObj->derivedOtherObjs.objects();

    myObj = findObjectById<SimpleObjDerivedOther*>( allObjects.begin(), allObjects.end(), s2Id );

    EXPECT_EQ( nullptr, myObj );
}
