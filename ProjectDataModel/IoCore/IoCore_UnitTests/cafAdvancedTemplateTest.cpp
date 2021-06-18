
#include "gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
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
        CAFFA_IO_InitField( &m_doubleField, "BigNumber" );
        auto doubleProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObjectA::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObjectA::doubleMember );
        m_doubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );
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
    double m_doubleMember;
};

CAFFA_IO_SOURCE_INIT( DemoObjectA, "DemoObjectA" );

namespace caffa
{
template <>
void AppEnum<DemoObjectA::TestEnumType>::setUp()
{
    addItem( DemoObjectA::T1, "T1", "An A letter" );
    addItem( DemoObjectA::T2, "T2", "A B letter" );
    addItem( DemoObjectA::T3, "T3", "A B letter" );
    setDefault( DemoObjectA::T1 );
}

} // namespace caffa

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

    std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string string = root->writeObjectToString( ioType );
        std::cout << string << std::endl;

        caffa::ObjectHandle* objCopy =
            caffa::ObjectIoCapability::readUnknownObjectFromString( string,
                                                                    caffa::DefaultObjectFactory::instance(),
                                                                    true,
                                                                    ioType );
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

        std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

        for ( auto ioType : ioTypes )
        {
            {
                auto        a              = std::make_unique<DemoObjectA>();
                auto        ap             = siblingPtr->m_demoObjs.push_back( std::move( a ) );
                std::string originalOutput = ap->writeObjectToString();
                {
                    auto objCopy = ap->capability<caffa::ObjectIoCapability>()
                                       ->copyBySerialization( caffa::DefaultObjectFactory::instance(), ioType );
                    auto demoObj = dynamic_cast<DemoObjectA*>( objCopy );

                    std::string copyOutput = ap->capability<caffa::ObjectIoCapability>()->writeObjectToString();

                    ASSERT_EQ( originalOutput, copyOutput );

                    delete objCopy;
                }

                {
                    auto objCopy = ap->capability<caffa::ObjectIoCapability>()
                                       ->copyBySerialization( caffa::DefaultObjectFactory::instance(), ioType );

                    auto demoObj = dynamic_cast<DemoObjectA*>( objCopy );
                    siblingPtr->m_demoObjs.push_back( std::unique_ptr<DemoObjectA>( demoObj ) );
                }
            }
        }
    }
}
