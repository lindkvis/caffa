
#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmFieldIoCapabilitySpecializations.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectHandleIoMacros.h"
#include "cafPdmObjectIoCapability.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"

#include <QXmlStreamWriter>

class ItemPdmObject : public caf::PdmObjectHandle, public caf::PdmObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    ItemPdmObject()
        : PdmObjectHandle()
        , PdmObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_name, "Name" );
    }

    explicit ItemPdmObject( QString name )
        : PdmObjectHandle()
        , PdmObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~ItemPdmObject() {}

    // Fields
    caf::PdmDataValueField<QString> m_name;
};
CAF_PDM_IO_SOURCE_INIT( ItemPdmObject, "ItemPdmObject" );

class DemoPdmObjectA;

class ContainerPdmObject : public caf::PdmObjectHandle, public caf::PdmObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    ContainerPdmObject()
        : PdmObjectHandle()
        , PdmObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_items, "Items" );
        CAF_PDM_IO_InitField( &m_containers, "Containers" );
        CAF_PDM_IO_InitField( &m_demoObjs, "DemoObjs" );
    }

    ~ContainerPdmObject() {}

    // Fields
    caf::PdmChildArrayField<ItemPdmObject*>      m_items;
    caf::PdmChildArrayField<ContainerPdmObject*> m_containers;
    caf::PdmChildArrayField<DemoPdmObjectA*>     m_demoObjs;
};
CAF_PDM_IO_SOURCE_INIT( ContainerPdmObject, "ContainerPdmObject" );

class DemoPdmObjectA : public caf::PdmObjectHandle, public caf::PdmObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoPdmObjectA()
        : PdmObjectHandle()
        , PdmObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_doubleField, "BigNumber" );
        m_doubleField.registerSetMethod( this, &DemoPdmObjectA::setDoubleMember );
        m_doubleField.registerGetMethod( this, &DemoPdmObjectA::doubleMember );

        CAF_PDM_IO_InitField( &m_pointerToItem, "TestPointerToItem" );
        CAF_PDM_IO_InitField( &m_pointerToDemoObj, "TestPointerToDemo" );
    }

    ~DemoPdmObjectA() {}

    // Fields
    caf::PdmProxyValueField<double>         m_doubleField;
    caf::PdmPtrField<caf::PdmObjectHandle*> m_pointerToItem;
    caf::PdmPtrField<caf::PdmObjectHandle*> m_pointerToDemoObj;

    caf::PdmDataValueField<caf::FilePath> m_singleFilePath;

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

CAF_PDM_IO_SOURCE_INIT( DemoPdmObjectA, "DemoPdmObjectA" );

namespace caf
{
template <>
void AppEnum<DemoPdmObjectA::TestEnumType>::setUp()
{
    addItem( DemoPdmObjectA::T1, "T1", "An A letter" );
    addItem( DemoPdmObjectA::T2, "T2", "A B letter" );
    addItem( DemoPdmObjectA::T3, "T3", "A B letter" );
    setDefault( DemoPdmObjectA::T1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a QString
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, FieldWrite )
{
    ContainerPdmObject* root      = new ContainerPdmObject;
    ContainerPdmObject* container = new ContainerPdmObject;
    ContainerPdmObject* sibling   = new ContainerPdmObject;
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj C";

        container->m_items.push_back( item );
    }

    std::vector<caf::PdmObjectIoCapability::IoParameters::IoType> ioTypes =
        { caf::PdmObjectIoCapability::IoParameters::IoType::XML, caf::PdmObjectIoCapability::IoParameters::IoType::JSON };

    // Test with empty ptr field
    for ( auto ioType : ioTypes )
    {
        QString serializedString;
        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back( a );
            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString.toStdString() << std::endl;
            delete a;
        }

        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance(), ioType );

            ASSERT_TRUE( a->m_pointerToItem() == NULL );
        }
    }

    for ( auto ioType : ioTypes )
    {
        QString serializedString;
        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back( a );

            a->m_pointerToItem = container->m_items[1];

            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString.toStdString() << std::endl;
            delete a;
        }

        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance(), ioType );
            a->capability<caf::PdmObjectIoCapability>()->resolveReferencesRecursively();

            ASSERT_TRUE( a->m_pointerToItem() == container->m_items[1] );
        }
    }

    for ( auto ioType : ioTypes )
    {
        QString string = root->writeObjectToString( ioType );
        std::cout << string.toStdString() << std::endl;

        caf::PdmObjectHandle* objCopy =
            caf::PdmObjectIoCapability::readUnknownObjectFromString( string,
                                                                     caf::PdmDefaultObjectFactory::instance(),
                                                                     true,
                                                                     ioType );
        auto rootCopy = dynamic_cast<ContainerPdmObject*>( objCopy );
        ASSERT_TRUE( rootCopy != nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AdvancedObjectTest, CopyOfObjects )
{
    ContainerPdmObject* root      = new ContainerPdmObject;
    ContainerPdmObject* container = new ContainerPdmObject;
    ContainerPdmObject* sibling   = new ContainerPdmObject;
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name        = "Obj C";

        container->m_items.push_back( item );

        std::vector<caf::PdmObjectIoCapability::IoParameters::IoType> ioTypes =
            { caf::PdmObjectIoCapability::IoParameters::IoType::XML,
              caf::PdmObjectIoCapability::IoParameters::IoType::JSON };

        for ( auto ioType : ioTypes )
        {
            {
                DemoPdmObjectA* a = new DemoPdmObjectA;
                sibling->m_demoObjs.push_back( a );

                a->m_pointerToItem = container->m_items[1];

                {
                    auto* objCopy = dynamic_cast<DemoPdmObjectA*>(
                        a->capability<caf::PdmObjectIoCapability>()
                            ->copyBySerialization( caf::PdmDefaultObjectFactory::instance(), ioType ) );
                    std::vector<caf::PdmFieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_FALSE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }

                {
                    auto* objCopy = dynamic_cast<DemoPdmObjectA*>(
                        a->capability<caf::PdmObjectIoCapability>()
                            ->copyBySerialization( caf::PdmDefaultObjectFactory::instance(), ioType ) );

                    sibling->m_demoObjs.push_back( objCopy );

                    std::vector<caf::PdmFieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_TRUE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }
            }
        }
    }
}
