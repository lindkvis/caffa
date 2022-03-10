
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"

class ItemObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    ItemObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_name, "Name" );
    }

    explicit ItemObject( std::string name )
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~ItemObject() {}

    std::string classKeywordDynamic() const override { return classKeyword(); }

    // Fields
    caffa::DataValueField<std::string> m_name;
};
CAFFA_IO_SOURCE_INIT( ItemObject, "ItemObject" );

class DemoObjectA;

class ContainerObject : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    ContainerObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_items, "Items" );
        CAFFA_IO_InitField( &m_containers, "Containers" );
        CAFFA_IO_InitField( &m_demoObjs, "DemoObjs" );
    }

    ~ContainerObject() {}

    std::string classKeywordDynamic() const override { return classKeyword(); }

    // Fields
    caffa::ChildArrayField<ItemObject*>      m_items;
    caffa::ChildArrayField<ContainerObject*> m_containers;
    caffa::ChildArrayField<DemoObjectA*>     m_demoObjs;
};
CAFFA_IO_SOURCE_INIT( ContainerObject, "ContainerObject" );

class DemoObjectA : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAFFA_IO_HEADER_INIT;

public:
    DemoObjectA()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAFFA_IO_InitField( &m_doubleField, "BigNumber" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObjectA::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObjectA::doubleMember );
        m_doubleField.setAccessor( std::move( doubleProxyAccessor ) );
    }

    ~DemoObjectA() {}

    // Fields
    caffa::DataValueField<double> m_doubleField;

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
    std::string classKeywordDynamic() const override { return classKeyword(); }

    double m_doubleMember;
};

CAFFA_IO_SOURCE_INIT( DemoObjectA, "DemoObjectA" );

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, FieldWrite )
{
    auto root         = std::make_unique<ContainerObject>();
    auto container    = std::make_unique<ContainerObject>();
    auto sibling      = std::make_unique<ContainerObject>();
    auto containerPtr = container.get();
    root->m_containers.push_back( std::move( container ) );
    auto siblingPtr = sibling.get();
    root->m_containers.push_back( std::move( sibling ) );

    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj A";

        containerPtr->m_items.push_back( std::move( item ) );
    }
    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj B";

        containerPtr->m_items.push_back( std::move( item ) );
    }

    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj C";

        containerPtr->m_items.push_back( std::move( item ) );
    }

    caffa::JsonSerializer serializer;
    std::string           string = serializer.writeObjectToString( root.get() );

    std::cout << string << std::endl;

    std::unique_ptr<caffa::ObjectHandle> objCopy  = serializer.createObjectFromString( string );
    auto                                 rootCopy = dynamic_cast<ContainerObject*>( objCopy.get() );
    ASSERT_TRUE( rootCopy != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, CopyOfObjects )
{
    auto root         = std::make_unique<ContainerObject>();
    auto container    = std::make_unique<ContainerObject>();
    auto sibling      = std::make_unique<ContainerObject>();
    auto containerPtr = container.get();
    root->m_containers.push_back( std::move( container ) );
    auto siblingPtr = sibling.get();
    root->m_containers.push_back( std::move( sibling ) );

    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj A";

        containerPtr->m_items.push_back( std::move( item ) );
    }
    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj B";

        containerPtr->m_items.push_back( std::move( item ) );
    }

    {
        auto item    = std::make_unique<ItemObject>();
        item->m_name = "Obj C";

        containerPtr->m_items.push_back( std::move( item ) );

        caffa::JsonSerializer serializer;

        {
            auto a  = std::make_unique<DemoObjectA>();
            auto ap = a.get();
            siblingPtr->m_demoObjs.push_back( std::move( a ) );
            std::string originalOutput = serializer.writeObjectToString( ap );
            {
                auto        objCopy    = serializer.copyBySerialization( ap );
                auto        demoObj    = caffa::dynamic_unique_cast<DemoObjectA>( std::move( objCopy ) );
                std::string copyOutput = serializer.writeObjectToString( demoObj.get() );
                ASSERT_EQ( originalOutput, copyOutput );
            }

            {
                auto objCopy = serializer.copyBySerialization( ap );
                siblingPtr->m_demoObjs.push_back( caffa::dynamic_unique_cast<DemoObjectA>( std::move( objCopy ) ) );
            }
        }
    }
}
