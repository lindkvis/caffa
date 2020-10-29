
#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"

#include <QXmlStreamWriter>

class ItemObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    ItemObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_name, "Name" );
    }

    explicit ItemObject( QString name )
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_name, "Name" );
        m_name = name;
    }

    ~ItemObject() {}

    // Fields
    caf::PdmDataValueField<QString> m_name;
};
CAF_PDM_IO_SOURCE_INIT( ItemObject, "ItemObject" );

class DemoObjectA;

class ContainerObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    ContainerObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_items, "Items" );
        CAF_PDM_IO_InitField( &m_containers, "Containers" );
        CAF_PDM_IO_InitField( &m_demoObjs, "DemoObjs" );
    }

    ~ContainerObject() {}

    // Fields
    caf::PdmChildArrayField<ItemObject*>      m_items;
    caf::PdmChildArrayField<ContainerObject*> m_containers;
    caf::PdmChildArrayField<DemoObjectA*>     m_demoObjs;
};
CAF_PDM_IO_SOURCE_INIT( ContainerObject, "ContainerObject" );

class DemoObjectA : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

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
        CAF_PDM_IO_InitField( &m_doubleField, "BigNumber" );
        m_doubleField.registerSetMethod( this, &DemoObjectA::setDoubleMember );
        m_doubleField.registerGetMethod( this, &DemoObjectA::doubleMember );

        CAF_PDM_IO_InitField( &m_pointerToItem, "TestPointerToItem" );
        CAF_PDM_IO_InitField( &m_pointerToDemoObj, "TestPointerToDemo" );
    }

    ~DemoObjectA() {}

    // Fields
    caf::PdmProxyValueField<double>         m_doubleField;
    caf::PdmPtrField<caf::ObjectHandle*> m_pointerToItem;
    caf::PdmPtrField<caf::ObjectHandle*> m_pointerToDemoObj;

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

CAF_PDM_IO_SOURCE_INIT( DemoObjectA, "DemoObjectA" );

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
/// Read/write fields to a valid Xml document encoded in a QString
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
        item->m_name        = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemObject* item = new ItemObject();
        item->m_name        = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemObject* item = new ItemObject();
        item->m_name        = "Obj C";

        container->m_items.push_back( item );
    }

    std::vector<caf::ObjectIoCapability::IoParameters::IoType> ioTypes =
        { caf::ObjectIoCapability::IoParameters::IoType::XML, caf::ObjectIoCapability::IoParameters::IoType::JSON };

    // Test with empty ptr field
    for ( auto ioType : ioTypes )
    {
        QString serializedString;
        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );
            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString.toStdString() << std::endl;
            delete a;
        }

        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance(), ioType );

            ASSERT_TRUE( a->m_pointerToItem() == NULL );
        }
    }

    for ( auto ioType : ioTypes )
    {
        QString serializedString;
        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->m_pointerToItem = container->m_items[1];

            serializedString = a->writeObjectToString( ioType );
            std::cout << serializedString.toStdString() << std::endl;
            delete a;
        }

        {
            DemoObjectA* a = new DemoObjectA;
            sibling->m_demoObjs.push_back( a );

            a->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance(), ioType );
            a->capability<caf::ObjectIoCapability>()->resolveReferencesRecursively();

            ASSERT_TRUE( a->m_pointerToItem() == container->m_items[1] );
        }
    }

    for ( auto ioType : ioTypes )
    {
        QString string = root->writeObjectToString( ioType );
        std::cout << string.toStdString() << std::endl;

        caf::ObjectHandle* objCopy =
            caf::ObjectIoCapability::readUnknownObjectFromString( string,
                                                                     caf::PdmDefaultObjectFactory::instance(),
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
    ContainerObject* root      = new ContainerObject;
    ContainerObject* container = new ContainerObject;
    ContainerObject* sibling   = new ContainerObject;
    root->m_containers.push_back( container );
    root->m_containers.push_back( sibling );

    {
        ItemObject* item = new ItemObject();
        item->m_name        = "Obj A";

        container->m_items.push_back( item );
    }
    {
        ItemObject* item = new ItemObject();
        item->m_name        = "Obj B";

        container->m_items.push_back( item );
    }

    {
        ItemObject* item = new ItemObject();
        item->m_name        = "Obj C";

        container->m_items.push_back( item );

        std::vector<caf::ObjectIoCapability::IoParameters::IoType> ioTypes =
            { caf::ObjectIoCapability::IoParameters::IoType::XML,
              caf::ObjectIoCapability::IoParameters::IoType::JSON };

        for ( auto ioType : ioTypes )
        {
            {
                DemoObjectA* a = new DemoObjectA;
                sibling->m_demoObjs.push_back( a );

                a->m_pointerToItem = container->m_items[1];

                {
                    auto* objCopy = dynamic_cast<DemoObjectA*>(
                        a->capability<caf::ObjectIoCapability>()
                            ->copyBySerialization( caf::PdmDefaultObjectFactory::instance(), ioType ) );
                    std::vector<caf::FieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively( &fieldWithFailingResolve );
                    ASSERT_FALSE( fieldWithFailingResolve.empty() );
                    delete objCopy;
                }

                {
                    auto* objCopy = dynamic_cast<DemoObjectA*>(
                        a->capability<caf::ObjectIoCapability>()
                            ->copyBySerialization( caf::PdmDefaultObjectFactory::instance(), ioType ) );

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
