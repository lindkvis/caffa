
#include "gtest/gtest.h"

#include "cafChildArrayField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafPtrArrayField.h"
#include "cafReferenceHelper.h"

class MyItemObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    MyItemObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_name, "Name" );
    }

    explicit MyItemObject( std::string name )
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~MyItemObject() {}

    // Fields
    caf::DataValueField<std::string> m_name;
};
CAF_IO_SOURCE_INIT( MyItemObject, "MyItemObject" );

class MyContainerObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    MyContainerObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_items, "Items" );
        CAF_IO_InitField( &m_containers, "Containers" );
    }

    ~MyContainerObject() {}

    // Fields
    caf::ChildArrayField<MyItemObject*> m_items;
    caf::PtrArrayField<MyItemObject*>   m_containers;
};
CAF_IO_SOURCE_INIT( MyContainerObject, "MyContainerObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PtrArrayBaseTest, PtrArraySerializeTest )
{
    MyContainerObject* objA = new MyContainerObject;

    MyItemObject* s1 = new MyItemObject;
    MyItemObject* s2 = new MyItemObject;
    MyItemObject* s3 = new MyItemObject;

    objA->m_items.push_back( s1 );
    objA->m_items.push_back( s2 );
    objA->m_items.push_back( s3 );

    objA->m_containers.push_back( s1 );
    objA->m_containers.push_back( s2 );
    objA->m_containers.push_back( s3 );

    // delete s2;

    std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            serializedString = objA->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
        }

        {
            MyContainerObject* ihd1 = new MyContainerObject;

            ihd1->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
            ihd1->resolveReferencesRecursively();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PtrArrayBaseTest, DeleteObjectPtrArraySerializeTest )
{
    MyContainerObject* objA = new MyContainerObject;

    MyItemObject* s1 = new MyItemObject;
    MyItemObject* s2 = new MyItemObject;
    MyItemObject* s3 = new MyItemObject;

    objA->m_items.push_back( s1 );
    objA->m_items.push_back( s2 );
    objA->m_items.push_back( s3 );

    objA->m_containers.push_back( s1 );
    objA->m_containers.push_back( s2 );
    objA->m_containers.push_back( s3 );

    delete s2;

    std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            serializedString = objA->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
        }

        {
            MyContainerObject* ihd1 = new MyContainerObject;

            ihd1->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
            ihd1->resolveReferencesRecursively();

            EXPECT_TRUE( ihd1->m_containers.at( 0 ) != nullptr );
            EXPECT_TRUE( ihd1->m_containers.at( 1 ) == nullptr ); // Deleted
            EXPECT_TRUE( ihd1->m_containers.at( 2 ) == nullptr ); // Pointing to item at index 2, does not longer exist

            EXPECT_TRUE( ihd1->m_items.size() == size_t( 2 ) );
            EXPECT_TRUE( ihd1->m_items[0] != nullptr );
            EXPECT_TRUE( ihd1->m_items[1] != nullptr );
        }
    }
}
