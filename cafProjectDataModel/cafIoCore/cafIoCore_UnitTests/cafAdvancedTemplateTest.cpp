
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
    ContainerObject* root      = new ContainerObject;
    ContainerObject* container = new ContainerObject;
    ContainerObject* sibling   = new ContainerObject;
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj C";

        container->m_items.push_back( item );
    }

    std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

    // Test with empty ptr field
    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );
            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString << std::endl;
            delete a;
        }

        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );

            ASSERT_TRUE( a->m_pointerToItem() == NULL );
        }
    }

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->m_pointerToItem = container->m_items[1];

            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString << std::endl;
            delete a;
        }

        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
            a->capability<caf::ObjectIoCapability>()->resolveReferencesRecursively();

            ASSERT_TRUE( a->m_pointerToItem() == container->m_items[1] );
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
    ContainerObject* root      = new ContainerObject;
    ContainerObject* container = new ContainerObject;
    ContainerObject* sibling   = new ContainerObject;
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemObject* item = new ItemObject();
        item->m_name     = "Obj C";

        container->m_items.push_back( item );

        std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

        for ( auto ioType : ioTypes )
        {
            {
                DemoObjectA* a = new DemoObjectA;
                sibling->m_demoObjs.push_back( a );

                a->m_pointerToItem = container->m_items[1];

                {
                    auto* objCopy = dynamic_cast<DemoObjectA*>(
                        a->capability<caf::ObjectIoCapability>()->copyBySerialization( caf::DefaultObjectFactory::instance(),
                                                                                       ioType ) );
                    std::vector<caf::FieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_FALSE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }

                {
                    auto* objCopy = dynamic_cast<DemoObjectA*>(
                        a->capability<caf::ObjectIoCapability>()->copyBySerialization( caf::DefaultObjectFactory::instance(),
                                                                                       ioType ) );

                    sibling->m_demoObjs.push_back( objCopy );

                    std::vector<caf::FieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_TRUE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }
            }
        }
    }
}
