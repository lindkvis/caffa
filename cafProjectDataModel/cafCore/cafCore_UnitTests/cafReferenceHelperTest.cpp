
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"
#include "cafReferenceHelper.h"

class SimpleObj : public caf::ObjectHandle
{
public:
    SimpleObj()
        : ObjectHandle()
        , m_doubleMember( 0.0 )
    {
        this->addField( &m_position, "m_position" );
        this->addField( &m_dir, "m_dir" );
        this->addField( &m_up, "m_up" );
        this->addField( &m_proxyDouble, "m_proxyDouble" );
    }

    caf::DataValueField<double> m_position;
    caf::DataValueField<double> m_dir;
    caf::DataValueField<double> m_up;
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

class ReferenceSimpleObj : public caf::ObjectHandle
{
public:
    ReferenceSimpleObj()
        : ObjectHandle()
    {
        this->addField( &m_pointersField, "m_pointersField" );
        this->addField( &m_simpleObjPtrField, "m_simpleObjPtrField" );
    }

    // Fields
    caf::ChildField<ObjectHandle*>   m_pointersField;
    caf::ChildArrayField<SimpleObj*> m_simpleObjPtrField;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FindRootFromObject )
{
    {
        caf::ObjectHandle* obj = NULL;
        EXPECT_EQ( NULL, caf::ReferenceHelper::findRoot( obj ) );
    }

    {
        caf::ObjectHandle* obj = new SimpleObj;
        EXPECT_EQ( obj, caf::ReferenceHelper::findRoot( obj ) );
        delete obj;
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );

        EXPECT_EQ( ihd1, caf::ReferenceHelper::findRoot( s1 ) );
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FindRootFromField )
{
    {
        caf::FieldHandle* fieldHandle = NULL;
        EXPECT_EQ( NULL, caf::ReferenceHelper::findRoot( fieldHandle ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );

        EXPECT_EQ( ihd1, caf::ReferenceHelper::findRoot( &s1->m_dir ) );
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFrommRootToField )
{
    {
        caf::ObjectHandle* obj         = NULL;
        caf::FieldHandle*  fieldHandle = NULL;
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( obj, fieldHandle ).empty() );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        SimpleObj*          s3   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );
        ihd1->m_simpleObjPtrField.push_back( s3 );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( NULL, &s3->m_dir ).empty() );
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( ihd1, NULL ).empty() );

        std::string refString      = caf::ReferenceHelper::referenceFromRootToField( ihd1, &s3->m_dir );
        std::string expectedString = "m_dir m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.c_str(), refString.c_str() );

        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFrommRootToObject )
{
    {
        caf::ObjectHandle* root = NULL;
        caf::ObjectHandle* obj  = NULL;
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( root, obj ).empty() );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        SimpleObj*          s3   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );
        ihd1->m_simpleObjPtrField.push_back( s3 );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( NULL, s3 ).empty() );
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( ihd1, NULL ).empty() );

        std::string refString      = caf::ReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        std::string expectedString = "m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.c_str(), refString.c_str() );

        ReferenceSimpleObj* ihd2 = new ReferenceSimpleObj;
        SimpleObj*          s4   = new SimpleObj;
        ihd2->m_simpleObjPtrField.push_back( s4 );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( ihd1, s4 ).empty() );

        delete ihd1;
        delete ihd2;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ObjectFromReference )
{
    {
        caf::ObjectHandle* root = NULL;
        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( root, "" ) );
        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( root, "a 2 b 4" ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( ihd1, "" ) );
        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( ihd1, "a 2 b 4" ) );

        std::string refString = caf::ReferenceHelper::referenceFromRootToObject( ihd1, s2 );
        EXPECT_EQ( s2, caf::ReferenceHelper::objectFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.removeChildObject( s2 );
        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.deleteAllChildObjects();

        EXPECT_EQ( NULL, caf::ReferenceHelper::objectFromReference( ihd1, refString ) );

        delete s2;
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FieldFromReference )
{
    {
        caf::ObjectHandle* root = NULL;
        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( root, "" ) );
        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( root, "a 2 b 4" ) );
    }

    {
        SimpleObj*          s1   = new SimpleObj;
        SimpleObj*          s2   = new SimpleObj;
        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( ihd1, "" ) );
        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( ihd1, "a 2 b 4" ) );

        caf::FieldHandle* fHandle = &s2->m_position;

        std::string refString = caf::ReferenceHelper::referenceFromRootToField( ihd1, fHandle );
        EXPECT_EQ( fHandle, caf::ReferenceHelper::fieldFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.removeChildObject( s2 );
        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( ihd1, refString ) );

        ihd1->m_simpleObjPtrField.deleteAllChildObjects();

        EXPECT_EQ( NULL, caf::ReferenceHelper::fieldFromReference( ihd1, refString ) );

        delete s2;
        delete ihd1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFromFieldToObject )
{
    {
        caf::ObjectHandle* root        = NULL;
        caf::FieldHandle*  fieldHandle = NULL;

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromFieldToObject( fieldHandle, root ).empty() );
    }

    {
        SimpleObj* s1 = new SimpleObj;
        SimpleObj* s2 = new SimpleObj;

        caf::FieldHandle* s2FieldHandle = &s2->m_dir;

        // Unrelated objects
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, s1 ).empty() );

        ReferenceSimpleObj* root    = new ReferenceSimpleObj;
        SimpleObj*          root_s1 = new SimpleObj;
        root->m_simpleObjPtrField.push_back( root_s1 );

        ReferenceSimpleObj* ihd1 = new ReferenceSimpleObj;
        root->m_pointersField    = ihd1;

        ihd1->m_simpleObjPtrField.push_back( s1 );
        ihd1->m_simpleObjPtrField.push_back( s2 );

        std::string        refString = caf::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, root_s1 );
        caf::ObjectHandle* obj       = caf::ReferenceHelper::objectFromFieldReference( s2FieldHandle, refString );
        EXPECT_EQ( root_s1, obj );

        delete root;
    }
}
