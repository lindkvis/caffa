
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObject.h"

#include <string>

class MsjSimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT( MsjSimpleObj, Object )

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

CAFFA_SOURCE_INIT( MsjSimpleObj )

class SimpleObjDerived : public MsjSimpleObj
{
    CAFFA_HEADER_INIT( SimpleObjDerived, MsjSimpleObj )

public:
    SimpleObjDerived()
        : MsjSimpleObj()
    {
        initField( valueA, "valueA" );
    }

    caffa::Field<int> valueA;
};

CAFFA_SOURCE_INIT( SimpleObjDerived )

class SimpleObjDerivedOther : public MsjSimpleObj
{
    CAFFA_HEADER_INIT( SimpleObjDerivedOther, MsjSimpleObj )

public:
    SimpleObjDerivedOther()
        : MsjSimpleObj()
    {
        initField( valueDouble, "valueDouble" );
    }

    caffa::Field<double> valueDouble;
};

CAFFA_SOURCE_INIT( SimpleObjDerivedOther )

class ContainerObj : public caffa::Object
{
    CAFFA_HEADER_INIT( ContainerObj, Object )

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

CAFFA_SOURCE_INIT( ContainerObj )

TEST( ChildArrayFieldHandle, DerivedObjects )
{
    auto containerObj = std::make_shared<ContainerObj>();

    auto s0 = std::make_shared<SimpleObjDerived>();
    auto s1 = std::make_shared<SimpleObjDerived>();
    auto s2 = std::make_shared<SimpleObjDerived>();

    containerObj->derivedObjs.push_back( s0 );
    containerObj->derivedObjs.push_back( s1 );
    containerObj->derivedObjs.push_back( s2 );

    auto allObjects = containerObj->derivedObjs.objects();

    auto it =
        std::find_if( allObjects.begin(), allObjects.end(), []( auto objectPtr ) { return objectPtr->id.value() == 2; } );

    auto myObj = it != allObjects.end() ? *it : nullptr;
    EXPECT_EQ( s2, myObj );

    it = std::find_if( allObjects.begin(), allObjects.end(), []( auto objectPtr ) { return objectPtr->id.value() == -1; } );

    myObj = it != allObjects.end() ? *it : nullptr;
    EXPECT_EQ( nullptr, myObj );
}

TEST( ChildArrayFieldHandle, DerivedOtherObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    auto s0 = std::make_shared<SimpleObjDerivedOther>();
    auto s1 = std::make_shared<SimpleObjDerivedOther>();
    auto s2 = std::make_shared<SimpleObjDerivedOther>();

    int s2Id = s2->id;

    containerObj->derivedOtherObjs.push_back( s0 );
    containerObj->derivedOtherObjs.push_back( s1 );
    containerObj->derivedOtherObjs.push_back( s2 );

    auto allObjects = containerObj->derivedOtherObjs.objects();

    auto it = std::find_if( allObjects.begin(),
                            allObjects.end(),
                            [s2Id]( auto objectPtr ) { return objectPtr->id.value() == s2Id; } );

    auto myObj = it != allObjects.end() ? *it : nullptr;

    EXPECT_EQ( s2, myObj );

    containerObj->derivedOtherObjs.removeChildObject( myObj );

    allObjects = containerObj->derivedOtherObjs.objects();

    it = std::find_if( allObjects.begin(),
                       allObjects.end(),
                       [s2Id]( auto objectPtr ) { return objectPtr->id.value() == s2Id; } );

    myObj = it != allObjects.end() ? *it : nullptr;

    EXPECT_EQ( nullptr, myObj );
}
