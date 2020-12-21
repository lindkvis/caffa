
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafObjectHandle.h"
#include "cafProxyValueField.h"
#include "cafPtrField.h"
#include "cafPdmReferenceHelper.h"

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
        name = std::string("Name" ) + std::to_string( id );
    }

    caf::DataValueField<std::string> name;
    caf::DataValueField<int>     id;
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

    ~ContainerObj()
    {
        derivedObjs.deleteAllChildObjects();
        derivedOtherObjs.deleteAllChildObjects();
    }

    caf::ChildArrayField<SimpleObjDerived*>      derivedObjs;
    caf::ChildArrayField<SimpleObjDerivedOther*> derivedOtherObjs;
};

template <class U, typename T>
U findObjectById( T start, T end, int id )
{
    for ( T it = start; it != end; it++ )
    {
        if ( id == it->p()->id() )
        {
            return it->p();
        }
    }

    return NULL;
}

TEST( ChildArrayFieldHandle, DerivedObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    SimpleObjDerived* s0 = new SimpleObjDerived;
    SimpleObjDerived* s1 = new SimpleObjDerived;
    SimpleObjDerived* s2 = new SimpleObjDerived;
    containerObj->derivedObjs.push_back( s0 );
    containerObj->derivedObjs.push_back( s1 );
    containerObj->derivedObjs.push_back( s2 );

    SimpleObjDerived* myObj =
        findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), 2 );
    EXPECT_EQ( s2, myObj );

    myObj = findObjectById<SimpleObjDerived*>( containerObj->derivedObjs.begin(), containerObj->derivedObjs.end(), -1 );
    EXPECT_EQ( NULL, myObj );

    delete containerObj;
}

TEST( ChildArrayFieldHandle, DerivedOtherObjects )
{
    ContainerObj* containerObj = new ContainerObj;

    SimpleObjDerivedOther* s0 = new SimpleObjDerivedOther;
    SimpleObjDerivedOther* s1 = new SimpleObjDerivedOther;
    SimpleObjDerivedOther* s2 = new SimpleObjDerivedOther;

    int s2Id = s2->id;

    containerObj->derivedOtherObjs.push_back( s0 );
    containerObj->derivedOtherObjs.push_back( s1 );
    containerObj->derivedOtherObjs.push_back( s2 );

    SimpleObjDerivedOther* myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                                           containerObj->derivedOtherObjs.end(),
                                                                           s2Id );
    EXPECT_EQ( s2, myObj );

    myObj = findObjectById<SimpleObjDerivedOther*>( containerObj->derivedOtherObjs.begin(),
                                                    containerObj->derivedOtherObjs.end(),
                                                    -1 );
    EXPECT_EQ( NULL, myObj );

    delete containerObj;
}
