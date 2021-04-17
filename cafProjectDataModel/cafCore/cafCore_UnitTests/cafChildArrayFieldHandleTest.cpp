
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"

#include <string>

class MsjSimpleObj : public caf::ObjectHandle
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

    caf::DataValueField<std::string> name;
    caf::DataValueField<int>         id;
};

class SimpleObjDerived : public MsjSimpleObj
{
public:
    SimpleObjDerived()
        : MsjSimpleObj()
    {
        this->addField( &valueA, "valueA" );
    }

    caf::DataValueField<int> valueA;
};

class SimpleObjDerivedOther : public MsjSimpleObj
{
public:
    SimpleObjDerivedOther()
        : MsjSimpleObj()
    {
        this->addField( &valueDouble, "valueDouble" );
    }

    caf::DataValueField<double> valueDouble;
};

class ContainerObj : public caf::ObjectHandle
{
public:
    ContainerObj()
        : ObjectHandle()
    {
        this->addField( &derivedObjs, "derivedObjs" );
        this->addField( &derivedOtherObjs, "derivedOtherObjs" );
    }

    ~ContainerObj() {}

    caf::ChildArrayField<SimpleObjDerived*>      derivedObjs;
    caf::ChildArrayField<SimpleObjDerivedOther*> derivedOtherObjs;
};

template <class U, typename T>
U findObjectById( T start, T end, int id )
{
    for ( T it = start; it != end; it++ )
    {
        if ( id == it->p()->id.value() )
        {
            return it->p();
        }
    }

    return NULL;
}

TEST( ChildArrayFieldHandle, DerivedObjects )
{
    auto containerObj = std::make_unique<ContainerObj>();

    auto s0 = std::make_unique<SimpleObjDerived>();
    auto s1 = std::make_unique<SimpleObjDerived>();
    auto s2 = std::make_unique<SimpleObjDerived>();

    containerObj->derivedObjs.push_back( std::move( s0 ) );
    containerObj->derivedObjs.push_back( std::move( s1 ) );
    auto s2p = containerObj->derivedObjs.push_back( std::move( s2 ) );

    SimpleObjDerived* myObj =
        findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), 2 );
    EXPECT_EQ( s2p, myObj );

    myObj = findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), -1 );
    EXPECT_EQ( NULL, myObj );
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
    auto s2p = containerObj->derivedOtherObjs.push_back( std::move( s2 ) );

    SimpleObjDerivedOther* myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                                           containerObj->derivedOtherObjs.end(),
                                                                           s2Id );
    EXPECT_EQ( s2p, myObj );

    myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                    containerObj->derivedOtherObjs.end(),
                                                    -1 );
    EXPECT_EQ( NULL, myObj );
}
