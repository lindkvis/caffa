
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldIoCapability.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafPtrField.h"
#include "cafReferenceHelper.h"

class DemoObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

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
        CAF_IO_InitField( &m_proxyDoubleField, "BigNumber" );

        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &DemoObject::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &DemoObject::doubleMember );
        m_proxyDoubleField.setFieldDataAccessor( std::move( doubleProxyAccessor ) );

        CAF_IO_InitField( &m_proxyEnumField, "AppEnum" );
        auto proxyEnumAccessor = std::make_unique<caf::FieldProxyAccessor<caf::AppEnum<TestEnumType>>>();
        proxyEnumAccessor->registerSetMethod( this, &DemoObject::setEnumMember );
        proxyEnumAccessor->registerGetMethod( this, &DemoObject::enumMember );
        m_proxyEnumField.setFieldDataAccessor( std::move( proxyEnumAccessor ) );

        m_enumMember = T1;
    }

    ~DemoObject() {}

    // Fields

    caf::DataValueField<double>                     m_proxyDoubleField;
    caf::DataValueField<caf::AppEnum<TestEnumType>> m_proxyEnumField;

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

    void                       setEnumMember( const caf::AppEnum<TestEnumType>& val ) { m_enumMember = val.value(); }
    caf::AppEnum<TestEnumType> enumMember() const { return m_enumMember; }

    double       m_doubleMember;
    TestEnumType m_enumMember;
};

CAF_IO_SOURCE_INIT( DemoObject, "DemoObject" );

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
    std::string serializedString;
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

        std::cout << serializedString << std::endl;

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
        destinationObjectGroup.readFields(xmlStream, caf::DefaultObjectFactory::instance(), NULL);

        DemoObject* a = dynamic_cast<DemoObject*>(destinationObjectGroup.objects[0]);

        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());
        ASSERT_EQ(DemoObject::T3, a->m_proxyEnumField());

    }
}
#endif
//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a std::string
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, FieldWrite )
{
    std::vector<caf::ObjectIoCapability::IoType> ioTypes = { caf::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string serializedString;
        {
            DemoObject* a = new DemoObject;

            a->m_proxyDoubleField.setValue( 2.5 );
            ASSERT_DOUBLE_EQ( 2.5, a->m_proxyDoubleField.value() );

            serializedString = a->writeObjectToString( ioType );

            std::cout << serializedString << std::endl;

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

            a->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance(), ioType );
        }
    }
}

class InheritedDemoObj : public DemoObject
{
    CAF_IO_HEADER_INIT;

public:
    InheritedDemoObj()
    {
        CAF_IO_InitField( &m_texts, "Texts" );
        CAF_IO_InitField( &m_childArrayField, "DemoObjectects" );
    }

    caf::DataValueField<std::string>  m_texts;
    caf::ChildArrayField<DemoObject*> m_childArrayField;
};
CAF_IO_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj" );

class SimpleObj : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    SimpleObj()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
        , m_doubleMember( 0.0 )
    {
        CAF_IO_InitField( &m_position, "Position" );
        CAF_IO_InitField( &m_dir, "Dir" );
        CAF_IO_InitField( &m_up, "Up" );

        CAF_IO_InitField( &m_proxyDouble, "m_proxyDouble" );
        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );
        m_proxyDouble.setFieldDataAccessor( std::move( doubleProxyAccessor ) );
    }

    caf::DataValueField<double> m_position;
    caf::DataValueField<double> m_dir;
    caf::DataValueField<int>    m_up;
    caf::DataValueField<double> m_proxyDouble;

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
CAF_IO_SOURCE_INIT( SimpleObj, "SimpleObj" );

class ReferenceDemoObject : public caf::ObjectHandle, public caf::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    ReferenceDemoObject()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_pointersField, "SimpleObjPtrField" );
        CAF_IO_InitField( &m_simpleObjPtrField2, "SimpleObjPtrField2" );
    }

    // Fields
    caf::ChildField<ObjectHandle*>   m_pointersField;
    caf::ChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_IO_SOURCE_INIT( ReferenceDemoObject, "ReferenceDemoObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReferenceHelper )
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
        std::string refString      = caf::ReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        std::string expectedString = ihd1->m_childArrayField.keyword() + " 3";
        EXPECT_STREQ( refString.c_str(), expectedString.c_str() );

        caf::ObjectHandle* fromRef = caf::ReferenceHelper::objectFromReference( ihd1, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    ReferenceDemoObject* objA = new ReferenceDemoObject;
    objA->m_pointersField     = ihd1;

    {
        std::string refString = caf::ReferenceHelper::referenceFromRootToObject( objA, s3 );

        caf::ObjectHandle* fromRef = caf::ReferenceHelper::objectFromReference( objA, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    // Test reference to field
    {
        std::string refString = caf::ReferenceHelper::referenceFromRootToField( objA, &( ihd1->m_childArrayField ) );

        caf::FieldHandle* fromRef = caf::ReferenceHelper::fieldFromReference( objA, refString );
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

    std::string serializedString;
    {
        InheritedDemoObj* ihd1 = new InheritedDemoObj;
        ihd1->m_childArrayField.push_back( s1 );
        ihd1->m_childArrayField.push_back( s2 );
        ihd1->m_childArrayField.push_back( s3 );

        serializedString = ihd1->writeObjectToString();

        std::cout << serializedString << std::endl;

        delete ihd1;
    }

    {
        InheritedDemoObj* ihd1 = new InheritedDemoObj;
        ASSERT_EQ( 0u, ihd1->m_childArrayField.size() );

        ihd1->readObjectFromString( serializedString, caf::DefaultObjectFactory::instance() );

        ASSERT_DOUBLE_EQ( 10, ihd1->m_childArrayField[0]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 20, ihd1->m_childArrayField[1]->m_proxyDoubleField.value() );
        ASSERT_DOUBLE_EQ( 30, ihd1->m_childArrayField[2]->m_proxyDoubleField.value() );
    }
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
        EXPECT_EQ( "double", dataTypeNameDouble );
    }

    {
        auto dataTypeNameDouble = s1->m_proxyDouble.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "double", dataTypeNameDouble );
    }

    {
        auto dataTypeNameDouble = s1->m_up.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( "int", dataTypeNameDouble );
    }

    {
        InheritedDemoObj* obj                = new InheritedDemoObj;
        auto              dataTypeNameDouble = obj->m_texts.capability<caf::FieldIoCapability>()->dataTypeName();
        EXPECT_EQ( typeid( std::string ).name(), dataTypeNameDouble );
    }

    delete s1;
}
#endif
