
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"
#include "cafReferenceHelper.h"

#include <memory>

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
        caf::ObjectHandle* obj = nullptr;
        EXPECT_EQ( nullptr, caf::ReferenceHelper::findRoot( obj ) );
    }

    {
        auto obj = std::make_unique<SimpleObj>();
        EXPECT_EQ( obj.get(), caf::ReferenceHelper::findRoot( obj.get() ) );
    }

    {
        auto s1      = std::make_unique<SimpleObj>();
        auto ihd1    = std::make_unique<ReferenceSimpleObj>();
        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );

        EXPECT_EQ( ihd1.get(), caf::ReferenceHelper::findRoot( raw_s1p ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FindRootFromField )
{
    {
        caf::FieldHandle* fieldHandle = nullptr;
        EXPECT_EQ( nullptr, caf::ReferenceHelper::findRoot( fieldHandle ) );
    }

    {
        auto s1      = std::make_unique<SimpleObj>();
        auto ihd1    = std::make_unique<ReferenceSimpleObj>();
        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );

        EXPECT_EQ( ihd1.get(), caf::ReferenceHelper::findRoot( &raw_s1p->m_dir ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFrommRootToField )
{
    {
        caf::ObjectHandle* obj         = nullptr;
        caf::FieldHandle*  fieldHandle = nullptr;
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( obj, fieldHandle ).empty() );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto s3   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );
        auto raw_s3p = ihd1->m_simpleObjPtrField.push_back( std::move( s3 ) );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( nullptr, &raw_s3p->m_dir ).empty() );
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToField( ihd1.get(), nullptr ).empty() );

        std::string refString      = caf::ReferenceHelper::referenceFromRootToField( ihd1.get(), &raw_s3p->m_dir );
        std::string expectedString = "m_dir m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.c_str(), refString.c_str() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFrommRootToObject )
{
    {
        caf::ObjectHandle* root = nullptr;
        caf::ObjectHandle* obj  = nullptr;
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( root, obj ).empty() );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto s3   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );
        auto raw_s3p = ihd1->m_simpleObjPtrField.push_back( std::move( s3 ) );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( nullptr, raw_s3p ).empty() );
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( ihd1.get(), nullptr ).empty() );

        std::string refString      = caf::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s3p );
        std::string expectedString = "m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.c_str(), refString.c_str() );

        auto ihd2    = std::make_unique<ReferenceSimpleObj>();
        auto s4      = std::make_unique<SimpleObj>();
        auto raw_s4p = ihd2->m_simpleObjPtrField.push_back( std::move( s4 ) );

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s4p ).empty() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ObjectFromReference )
{
    {
        caf::ObjectHandle* root = nullptr;
        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( root, "" ) );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( root, "a 2 b 4" ) );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( ihd1.get(), "" ) );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( ihd1.get(), "a 2 b 4" ) );

        std::string refString = caf::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s2p );
        EXPECT_EQ( raw_s2p, caf::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );

        std::unique_ptr<caf::ObjectHandle> removedObject = ihd1->m_simpleObjPtrField.remove( raw_s2p );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );

        ihd1->m_simpleObjPtrField.clear();

        EXPECT_EQ( nullptr, caf::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FieldFromReference )
{
    {
        caf::ObjectHandle* root = nullptr;
        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( root, "" ) );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( root, "a 2 b 4" ) );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( ihd1.get(), "" ) );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( ihd1.get(), "a 2 b 4" ) );

        caf::FieldHandle* fHandle = &raw_s2p->m_position;

        std::string refString = caf::ReferenceHelper::referenceFromRootToField( ihd1.get(), fHandle );
        EXPECT_EQ( fHandle, caf::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );

        std::unique_ptr<caf::ObjectHandle> removedObject = ihd1->m_simpleObjPtrField.remove( raw_s2p );
        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );

        ihd1->m_simpleObjPtrField.clear();

        EXPECT_EQ( nullptr, caf::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFromFieldToObject )
{
    {
        caf::ObjectHandle* root        = nullptr;
        caf::FieldHandle*  fieldHandle = nullptr;

        EXPECT_TRUE( caf::ReferenceHelper::referenceFromFieldToObject( fieldHandle, root ).empty() );
    }

    {
        auto s1 = std::make_unique<SimpleObj>();
        auto s2 = std::make_unique<SimpleObj>();

        caf::FieldHandle* s2FieldHandle = &s2->m_dir;

        // Unrelated objects
        EXPECT_TRUE( caf::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, s1.get() ).empty() );

        auto root    = std::make_unique<ReferenceSimpleObj>();
        auto root_s1 = std::make_unique<SimpleObj>();
        root->m_simpleObjPtrField.push_back( std::move( root_s1 ) );

        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        root->m_pointersField = std::move( ihd1 );

        std::string        refString = caf::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, root_s1.get() );
        caf::ObjectHandle* obj       = caf::ReferenceHelper::objectFromFieldReference( s2FieldHandle, refString );
        EXPECT_EQ( root_s1.get(), obj );
    }
}
