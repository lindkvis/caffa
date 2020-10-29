
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapability.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafProxyValueField.h"
#include "cafPtrField.h"
#include "cafPdmReferenceHelper.h"

#include <QXmlStreamWriter>

class DemoObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    DemoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_proxyDoubleField, "BigNumber" );
        m_proxyDoubleField.registerSetMethod( this, &DemoObject::setDoubleMember );
        m_proxyDoubleField.registerGetMethod( this, &DemoObject::doubleMember );

        CAF_PDM_IO_InitField( &m_proxyEnumField, "AppEnum" );
        m_proxyEnumField.registerSetMethod( this, &DemoObject::setEnumMember );
        m_proxyEnumField.registerGetMethod( this, &DemoObject::enumMember );
        m_enumMember = T1;
    }

    ~DemoObject() {}

    // Fields

    caf::ProxyValueField<double>                     m_proxyDoubleField;
    caf::ProxyValueField<caf::AppEnum<TestEnumType>> m_proxyEnumField;

private:
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

    void                       setEnumMember( const caf::AppEnum<TestEnumType>& val ) { m_enumMember = val; }
    caf::AppEnum<TestEnumType> enumMember() const { return m_enumMember; }

    double       m_doubleMember;
    TestEnumType m_enumMember;
};

CAF_PDM_IO_SOURCE_INIT( DemoObject, "DemoObject" );

namespace caf
{
template <>
void AppEnum<DemoObject::TestEnumType>::setUp()
{
    addItem( DemoObject::T1, "T1", "An A letter" );
    addItem( DemoObject::T2, "T2", "A B letter" );
    addItem( DemoObject::T3, "T3", "A B letter" );
    setDefault( DemoObject::T1 );
}

} // namespace caf

TEST( BaseTest, Delete )
{
    DemoObject* s2 = new DemoObject;
    delete s2;
}

#if 0 
//--------------------------------------------------------------------------------------------------
/// Read/write Xml using ObjectGroup
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, Start)
{
    QString serializedString;
    {
        DemoObject* a = new DemoObject;

        a->m_proxyDoubleField.setValue(2.5);
        a->m_proxyEnumField.setValue(DemoObject::T3);

        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());

        caf::ObjectGroup objGroup;
        objGroup.addObject(a);

        QXmlStreamWriter xmlStream(&serializedString);
        xmlStream.setAutoFormatting(true);
        objGroup.writeFields(xmlStream, NULL);

        std::cout << serializedString.toStdString() << std::endl;

        delete a;
    }

    /*
        <Objects>
          <DemoObject>
            <BigNumber>2.5</BigNumber>
            <TestEnumValue>T3</TestEnumValue>
          </DemoObject>
        </Objects>
    */

    {
        caf::ObjectGroup destinationObjectGroup;
        QXmlStreamReader xmlStream(serializedString);
        destinationObjectGroup.readFields(xmlStream, caf::PdmDefaultObjectFactory::instance(), NULL);

        DemoObject* a = dynamic_cast<DemoObject*>(destinationObjectGroup.objects[0]);

        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());
        ASSERT_EQ(DemoObject::T3, a->m_proxyEnumField());

    }
}
#endif
//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a QString
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, FieldWrite )
{
    std::vector<caf::ObjectIoCapability::IoParameters::IoType> ioTypes =
        { caf::ObjectIoCapability::IoParameters::IoType::XML, caf::ObjectIoCapability::IoParameters::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        QString serializedString;
        {
            DemoObject* a = new DemoObject;

            a->m_proxyDoubleField.setValue( 2.5 );
            ASSERT_DOUBLE_EQ( 2.5, a->m_proxyDoubleField.value() );

            serializedString = a->writeObjectToString( ioType );

            std::cout << serializedString.toStdString() << std::endl;

            delete a;
        }

        /*
        <DemoObject>
            <BigNumber>2.5</BigNumber>
            <TestEnumValue>T3</TestEnumValue>
        </DemoObject>
        */

        {
            DemoObject* a = new DemoObject;

            a->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance(), ioType );
        }
    }
}

class InheritedDemoObj : public DemoObject
{
    CAF_PDM_IO_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        CAF_PDM_IO_InitField( &m_texts, "Texts" );
        CAF_PDM_IO_InitField( &m_childArrayField, "DemoObjectects" );
    }

    ~InheritedDemoObj() { m_childArrayField.deleteAllChildObjects(); }

    caf::DataValueField<QString>         m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
};
CAF_PDM_IO_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj" );

class SimpleObj : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    SimpleObj()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
        , m_doubleMember( 0.0 )
    {
        CAF_PDM_IO_InitField( &m_position, "Position" );
        CAF_PDM_IO_InitField( &m_dir, "Dir" );
        CAF_PDM_IO_InitField( &m_up, "Up" );

        CAF_PDM_IO_InitField( &m_singleFilePath, "m_singleFilePath" );
        CAF_PDM_IO_InitField( &m_multipleFilePath, "m_multipleFilePath" );

        CAF_PDM_IO_InitField( &m_proxyDouble, "m_proxyDouble" );
        m_proxyDouble.registerSetMethod( this, &SimpleObj::setDoubleMember );
        m_proxyDouble.registerGetMethod( this, &SimpleObj::doubleMember );
    }

    caf::DataValueField<double>  m_position;
    caf::DataValueField<double>  m_dir;
    caf::DataValueField<int>     m_up;
    caf::ProxyValueField<double> m_proxyDouble;

    caf::DataValueField<caf::FilePath>              m_singleFilePath;
    caf::DataValueField<std::vector<caf::FilePath>> m_multipleFilePath;

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
CAF_PDM_IO_SOURCE_INIT( SimpleObj, "SimpleObj" );

class ReferenceDemoObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    ReferenceDemoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_pointersField, "SimpleObjPtrField" );
        CAF_PDM_IO_InitField( &m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    ~ReferenceDemoObject()
    {
        delete m_pointersField();
        m_simpleObjPtrField2.deleteAllChildObjects();
    }

    // Fields
    caf::ChildField<ObjectHandle*> m_pointersField;
    caf::ChildArrayField<SimpleObj*>  m_simpleObjPtrField2;
};

CAF_PDM_IO_SOURCE_INIT( ReferenceDemoObject, "ReferenceDemoObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmReferenceHelper )
{
    DemoObject* s1 = new DemoObject;
    DemoObject* s2 = new DemoObject;
    DemoObject* s3 = new DemoObject;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_childArrayField.push_back( new DemoObject );

    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    {
        QString refString      = caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        QString expectedString = ihd1->m_childArrayField.keyword() + " 3";
        EXPECT_STREQ( refString.toLatin1(), expectedString.toLatin1() );

        caf::ObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference( ihd1, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    ReferenceDemoObject* objA = new ReferenceDemoObject;
    objA->m_pointersField        = ihd1;

    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject( objA, s3 );

        caf::ObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference( objA, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    // Test reference to field
    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToField( objA, &( ihd1->m_childArrayField ) );

        caf::FieldHandle* fromRef = caf::PdmReferenceHelper::fieldFromReference( objA, refString );
        EXPECT_TRUE( fromRef == &( ihd1->m_childArrayField ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ChildArrayFieldSerializing )
{
    DemoObject* s1 = new DemoObject;
    s1->m_proxyDoubleField.setValue( 10 );

    DemoObject* s2 = new DemoObject;
    s2->m_proxyDoubleField.setValue( 20 );

    DemoObject* s3 = new DemoObject;
    s3->m_proxyDoubleField.setValue( 30 );

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_childArrayField.push_back( s1 );
    ihd1->m_childArrayField.push_back( s2 );
    ihd1->m_childArrayField.push_back( s3 );

    QString serializedString;
    {
        serializedString = ihd1->writeObjectToString();

        std::cout << serializedString.toStdString() << std::endl;

        delete ihd1;
    }

    {
        InheritedDemoObj* ihd1 = new InheritedDemoObj;
        ASSERT_EQ( 0u, ihd1->m_childArrayField.size() );

        QXmlStreamReader xmlStream( serializedString );

        ihd1->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance() );

        ASSERT_DOUBLE_EQ( 10, ihd1->m_childArrayField[0]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 20, ihd1->m_childArrayField[1]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 30, ihd1->m_childArrayField[2]->m_proxyDoubleField.value() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Testing that the QXmlStreamReader actually can not just read a list of fields.
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, QXMLStreamTest )
{
    QString xmlText =
        //"<DemoObject>"
        "<BigNumber>2.5</BigNumber>"
        "<TestEnumValue>T3</TestEnumValue>"
        "<TestEnumValue2>T3</TestEnumValue2>"
        //"</DemoObject>"
        ;

    QXmlStreamReader inputStream( xmlText );

    QXmlStreamReader::TokenType tt;
    while ( !inputStream.atEnd() )
    {
        tt = inputStream.readNext();
        std::cout << inputStream.name().toString().toStdString() << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, FilePathSerializing )
{
    SimpleObj* s1 = new SimpleObj;

    QString newVal = "path with space";
    s1->m_multipleFilePath.v().push_back( newVal );
    s1->m_multipleFilePath.v().push_back( newVal );

    s1->m_singleFilePath = newVal;

    QString serializedString = s1->writeObjectToString();

    {
        SimpleObj* ihd1 = new SimpleObj;

        QXmlStreamReader xmlStream( serializedString );

        ihd1->readObjectFromString( serializedString, caf::PdmDefaultObjectFactory::instance() );

        EXPECT_EQ( 2u, ihd1->m_multipleFilePath.v().size() );
        EXPECT_EQ( newVal.toStdString(), ihd1->m_singleFilePath().path().toStdString() );

        delete ihd1;
    }

    delete s1;
}

// Type deduction is different on other platforms than Windows
#ifdef WIN32
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, TestDataType )
{
    SimpleObj* s1 = new SimpleObj;

    {
        auto dataTypeNameDouble = s1->m_position.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "double", dataTypeNameDouble.toStdString() );
    }

    {
        auto dataTypeNameDouble = s1->m_proxyDouble.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "double", dataTypeNameDouble.toStdString() );
    }

    {
        auto dataTypeNameDouble = s1->m_up.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "int", dataTypeNameDouble.toStdString() );
    }

    {
        auto dataTypeNameDouble = s1->m_singleFilePath.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "class caf::FilePath", dataTypeNameDouble.toStdString() );
    }

    {
        InheritedDemoObj* obj                = new InheritedDemoObj;
        auto              dataTypeNameDouble = obj->m_texts.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "class QString", dataTypeNameDouble.toStdString() );
    }

    delete s1;
}
#endif