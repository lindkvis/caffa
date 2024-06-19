
#include "gtest/gtest.h"

#include "cafChildArrayField.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"

#include <functional>

using namespace std::placeholders;

class ItemObject : public caffa::Object
{
    CAFFA_HEADER_INIT( ItemObject, Object )

public:
    ItemObject() { initField( m_name, "Name" ); }

    explicit ItemObject( std::string name )
    {
        initField( m_name, "Name" );
        m_name = name;
    }

    ~ItemObject() {}

    // Fields
    caffa::Field<std::string> m_name;
};
CAFFA_SOURCE_INIT( ItemObject )

class DemoObjectA;

class ContainerObject : public caffa::Object
{
    CAFFA_HEADER_INIT( ContainerObject, Object )

public:
    ContainerObject()
    {
        initField( m_items, "Items" );
        initField( m_containers, "Containers" );
        initField( m_demoObjs, "DemoObjs" );
    }

    ~ContainerObject() {}

    // Fields
    caffa::ChildArrayField<ItemObject*>      m_items;
    caffa::ChildArrayField<ContainerObject*> m_containers;
    caffa::ChildArrayField<DemoObjectA*>     m_demoObjs;
};
CAFFA_SOURCE_INIT( ContainerObject )

class DemoObjectA : public caffa::Object
{
    CAFFA_HEADER_INIT( DemoObjectA, Object )

public:
    DemoObjectA()
    {
        initField( m_doubleField, "BigNumber" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( std::bind( &DemoObjectA::setDoubleMember, this, _1 ) );
        doubleProxyAccessor->registerGetMethod( std::bind( &DemoObjectA::doubleMember, this ) );
        m_doubleField.setAccessor( std::move( doubleProxyAccessor ) );
    }

    ~DemoObjectA() {}

    // Fields
    caffa::Field<double> m_doubleField;

    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    double m_doubleMember;
};

CAFFA_SOURCE_INIT( DemoObjectA )

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, FieldWrite )
{
    auto root      = std::make_shared<ContainerObject>();
    auto container = std::make_shared<ContainerObject>();
    auto sibling   = std::make_shared<ContainerObject>();
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj A";

        container->m_items.push_back( item );
    }
    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj B";

        container->m_items.push_back( item );
    }

    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj C";

        container->m_items.push_back( item );
    }

    caffa::JsonSerializer serializer;
    std::string           string = serializer.writeObjectToString( root.get() );

    std::cout << string << std::endl;

    std::shared_ptr<caffa::ObjectHandle> objCopy = serializer.createObjectFromString( string );
    ASSERT_TRUE( std::dynamic_pointer_cast<ContainerObject>( objCopy ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, CopyOfObjects )
{
    auto root      = std::make_shared<ContainerObject>();
    auto container = std::make_shared<ContainerObject>();
    auto sibling   = std::make_shared<ContainerObject>();
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj A";

        container->m_items.push_back( item );
    }
    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj B";

        container->m_items.push_back( item );
    }

    {
        auto item    = std::make_shared<ItemObject>();
        item->m_name = "Obj C";

        container->m_items.push_back( item );

        caffa::JsonSerializer serializer;

        {
            auto a = std::make_shared<DemoObjectA>();
            sibling->m_demoObjs.push_back( a );
            std::string originalOutput = serializer.writeObjectToString( a.get() );
            {
                auto        objCopy    = serializer.copyBySerialization( a.get() );
                std::string copyOutput = serializer.writeObjectToString( objCopy.get() );
                ASSERT_EQ( originalOutput, copyOutput );
            }

            {
                auto objCopy = serializer.copyBySerialization( a.get() );
                sibling->m_demoObjs.push_back( std::dynamic_pointer_cast<DemoObjectA>( objCopy ) );
            }
        }
    }
}
