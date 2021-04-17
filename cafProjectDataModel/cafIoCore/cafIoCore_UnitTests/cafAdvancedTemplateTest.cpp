
#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafPtrField.h"
#include "cafReferenceHelper.h"

class ItemObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    ItemObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_name, "Name" );
    }

    explicit ItemObject( std::string name )
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~ItemObject() {}

    // Fields
    caf::DataValueField<std::string> m_name;
};
CAF_IO_SOURCE_INIT( ItemObject, "ItemObject" );

class DemoObjectA;

class ContainerObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    ContainerObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_items, "Items" );
        CAF_IO_InitField( &m_containers, "Containers" );
        CAF_IO_InitField( &m_demoObjs, "DemoObjs" );
    }

    ~ContainerObject() {}

    // Fields
    caf::ChildArrayField<ItemObject*>      m_items;
    caf::ChildArrayField<ContainerObject*> m_containers;
    caf::ChildArrayField<DemoObjectA*>     m_demoObjs;
};
CAF_IO_SOURCE_INIT( ContainerObject, "ContainerObject" );

class DemoObjectA : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoObjectA()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_doubleField, "BigNumber" );
        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObjectA::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObjectA::doubleMember );
        m_doubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );

        CAF_IO_InitField( &m_pointerToItem, "TestPointerToItem" );
        CAF_IO_InitField( &m_pointerToDemoObj, "TestPointerToDemo" );
    }

    ~DemoObjectA() {}

    // Fields
    caf::DataValueField<double>       m_doubleField;
    caf::PtrField<caf::ObjectHandle*> m_pointerToItem;
    caf::PtrField<caf::ObjectHandle*> m_pointerToDemoObj;

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

CAF_IO_SOURCE_INIT( DemoObjectA, "DemoObjectA" );

namespace caf
{
template <>
void AppEnum<DemoObjectA::TestEnumType>::setUp()
{
    addItem( DemoObjectA::T1, "T1", "An A letter" );
    addItem( DemoObjectA::T2, "T2", "A B letter" );
    addItem( DemoObjectA::T3, "T3", "A B letter" );
    setDefault( DemoObjectA::T1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, FieldWrite )
{
    auto root         = std::make_unique<ContainerObject>();
    auto container    = std::make_unique<ContainerObject>();
    auto sibling      = std::make_unique<ContainerObject>();
    auto containerPtr = root->m_containers.push_back( std::move( container ) );
    auto siblingPtr   = root->m_containers.push_back( std::move( sibling ) );

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

    std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

    // Test with empty ptr field
    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            auto a           = std::make_unique<DemoObjectA>();
            auto ap          = siblingPtr->m_demoObjs.push_back( std::move( a ) );
            serializedString = ap->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
        }

        {
            auto a  = std::make_unique<DemoObjectA>();
            auto ap = siblingPtr->m_demoObjs.push_back( std::move( a ) );
            ap->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
            ap->capability<caf::ObjectIoCapability>()->resolveReferencesRecursively();

            ASSERT_TRUE( ap->m_pointerToItem() == NULL );
        }
    }

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            auto a = std::make_unique<DemoObjectA>();

            a->m_pointerToItem = containerPtr->m_items[1];
            auto ap            = siblingPtr->m_demoObjs.push_back( std::move( a ) );

            serializedString = ap->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;
            ASSERT_TRUE( ap->m_pointerToItem() == containerPtr->m_items[1] );
        }

        {
            auto a  = std::make_unique<DemoObjectA>();
            auto ap = siblingPtr->m_demoObjs.push_back( std::move( a ) );

            ap->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
            ap->capability<caf::ObjectIoCapability>()->resolveReferencesRecursively();

            ASSERT_TRUE( ap->m_pointerToItem() == containerPtr->m_items[1] );
        }
    }

    for ( auto ioType : ioTypes )
    {
        std::string string = root->writeObjectToString( ioType );
        std::cout << string << std::endl;

        caf::ObjectHandle* objCopy =
            caf::ObjectIoCapability::readUnknownObjectFromString( string, caf::DefaultObjectFactory::instance(), true, ioType );
        auto rootCopy = dynamic_cast<ContainerObject*>( objCopy );
        ASSERT_TRUE( rootCopy != nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, CopyOfObjects )
{
    auto root         = std::make_unique<ContainerObject>();
    auto container    = std::make_unique<ContainerObject>();
    auto sibling      = std::make_unique<ContainerObject>();
    auto containerPtr = root->m_containers.push_back( std::move( container ) );
    auto siblingPtr   = root->m_containers.push_back( std::move( sibling ) );

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

        std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

        for ( auto ioType : ioTypes )
        {
            {
                auto a  = std::make_unique<DemoObjectA>();
                auto ap = siblingPtr->m_demoObjs.push_back( std::move( a ) );

                ap->m_pointerToItem = containerPtr->m_items[1];

                {
                    auto objCopy = ap->capability<caf::ObjectIoCapability>()
                                       ->copyBySerialization( caf::DefaultObjectFactory::instance(), ioType );
                    auto                           demoObj = dynamic_cast<DemoObjectA*>( objCopy );
                    std::vector<caf::FieldHandle*> fieldWithFailingResolve;
                    demoObj->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_FALSE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }

                {
                    auto objCopy = ap->capability<caf::ObjectIoCapability>()
                                       ->copyBySerialization( caf::DefaultObjectFactory::instance(), ioType );

                    auto demoObj = dynamic_cast<DemoObjectA*>( objCopy );
                    siblingPtr->m_demoObjs.push_back( std::unique_ptr<DemoObjectA>( demoObj ) );

                    std::vector<caf::FieldHandle*> fieldWithFailingResolve;
                    demoObj->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_TRUE( fieldWithFailingResolve.empty() );
                }
            }
        }
    }
}
