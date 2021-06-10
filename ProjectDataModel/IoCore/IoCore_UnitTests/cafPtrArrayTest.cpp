
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafPtrArrayField.h"
#include "cafReferenceHelper.h"

class MyItemObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    MyItemObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_name, "Name" );
    }

    explicit MyItemObject( std::string name )
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~MyItemObject() {}

    // Fields
    caffa::DataValueField<std::string> m_name;
};
CAFFA_IO_SOURCE_INIT( MyItemObject, "MyItemObject" );

class MyContainerObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    MyContainerObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_items, "Items" );
        CAFFA_IO_InitField( &m_containers, "Containers" );
    }

    ~MyContainerObject() {}

    // Fields
    caffa::ChildArrayField<MyItemObject*> m_items;
    caffa::PtrArrayField<MyItemObject*>   m_containers;
};
CAFFA_IO_SOURCE_INIT( MyContainerObject, "MyContainerObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PtrArrayBaseTest, PtrArraySerializeTest )
{
    MyContainerObject* objA = new MyContainerObject;

    auto s1 = std::make_unique<MyItemObject>();
    auto s2 = std::make_unique<MyItemObject>();
    auto s3 = std::make_unique<MyItemObject>();

    auto s1p = objA->m_items.push_back( std::move( s1 ) );
    auto s2p = objA->m_items.push_back( std::move( s2 ) );
    auto s3p = objA->m_items.push_back( std::move( s3 ) );

    objA->m_containers.push_back( s1p );
    objA->m_containers.push_back( s2p );
    objA->m_containers.push_back( s3p );

    // delete s2;

    std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            serializedString = objA->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
        }

        {
            MyContainerObject* ihd1 = new MyContainerObject;

            ihd1->readObjectFromString( serializedString, caffa::DefaultObjectFactory::instance(), ioType );
            ihd1->resolveReferencesRecursively();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PtrArrayBaseTest, DeleteObjectPtrArraySerializeTest )
{
    auto objA = std::make_unique<MyContainerObject>();

    auto s1 = std::make_unique<MyItemObject>();
    auto s2 = std::make_unique<MyItemObject>();
    auto s3 = std::make_unique<MyItemObject>();

    auto s1p = objA->m_items.push_back( std::move( s1 ) );
    auto s2p = objA->m_items.push_back( std::move( s2 ) );
    auto s3p = objA->m_items.push_back( std::move( s3 ) );

    objA->m_containers.push_back( s1p );
    objA->m_containers.push_back( s2p );
    objA->m_containers.push_back( s3p );

    {
        auto s2n = objA->m_items.remove( s2p.p() );
    }
    std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            serializedString = objA->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
        }

        {
            MyContainerObject* ihd1 = new MyContainerObject;

            ihd1->readObjectFromString( serializedString, caffa::DefaultObjectFactory::instance(), ioType );
            ihd1->resolveReferencesRecursively();
            EXPECT_EQ( (size_t)3, ihd1->m_containers.size() );
            if ( ihd1->m_containers.size() == 3u )
            {
                EXPECT_TRUE( ihd1->m_containers.at( 0 ) != nullptr );
                EXPECT_TRUE( ihd1->m_containers.at( 1 ) == nullptr ); // Deleted
            }
        }
    }
}
