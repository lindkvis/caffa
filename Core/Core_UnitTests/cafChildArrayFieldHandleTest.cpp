
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"

#include <string>

class MsjSimpleObj : public caffa::ObjectHandle
{
public:
    MsjSimpleObj()
        : ObjectHandle()
    {
        this->addField( &name, "Name" );
        this->addField( &id, "ID" );

        static int a = 0;

        id   = a++;
        name = std::string( "Name" ) + std::to_string( id );
    }

    caffa::Field<std::string> name;
    caffa::Field<int>         id;

    std::string classKeywordDynamic() const override { return "SimpleObj"; }
};

class SimpleObjDerived : public MsjSimpleObj
{
public:
    SimpleObjDerived()
        : MsjSimpleObj()
    {
        this->addField( &valueA, "valueA" );
    }

    caffa::Field<int> valueA;
};

class SimpleObjDerivedOther : public MsjSimpleObj
{
public:
    SimpleObjDerivedOther()
        : MsjSimpleObj()
    {
        this->addField( &valueDouble, "valueDouble" );
    }

    caffa::Field<double> valueDouble;
};

class ContainerObj : public caffa::ObjectHandle
{
public:
    ContainerObj()
        : ObjectHandle()
    {
        this->addField( &derivedObjs, "derivedObjs" );
        this->addField( &derivedOtherObjs, "derivedOtherObjs" );
    }

    ~ContainerObj() {}

    std::string classKeywordDynamic() const override { return "ContainerObj"; }

    caffa::ChildArrayField<SimpleObjDerived*>      derivedObjs;
    caffa::ChildArrayField<SimpleObjDerivedOther*> derivedOtherObjs;
};

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

    auto allObjects = containerObj->derivedObjs.value();

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

    auto allObjects = containerObj->derivedOtherObjs.value();

    SimpleObjDerivedOther* myObj = findObjectById<SimpleObjDerivedOther*>( allObjects.begin(), allObjects.end(), s2Id );

    EXPECT_EQ( s2p, myObj );

    containerObj->derivedOtherObjs.removeChildObject( myObj );

    allObjects = containerObj->derivedOtherObjs.value();

    myObj = findObjectById<SimpleObjDerivedOther*>( allObjects.begin(), allObjects.end(), s2Id );

    EXPECT_EQ( nullptr, myObj );
}
