
#include "gtest.h"

#include "cafAppEnum.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObjectHandle.h"
#include "cafPtrField.h"
#include "cafReferenceHelper.h"

#include <memory>

class SimpleObj : public caffa::ObjectHandle
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

    caffa::DataValueField<double> m_position;
    caffa::DataValueField<double> m_dir;
    caffa::DataValueField<double> m_up;
    caffa::DataValueField<double> m_proxyDouble;

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

class ReferenceSimpleObj : public caffa::ObjectHandle
{
public:
    ReferenceSimpleObj()
        : ObjectHandle()
    {
        this->addField( &m_pointersField, "m_pointersField" );
        this->addField( &m_simpleObjPtrField, "m_simpleObjPtrField" );
    }

    // Fields
    caffa::ChildField<ObjectHandle*>   m_pointersField;
    caffa::ChildArrayField<SimpleObj*> m_simpleObjPtrField;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FindRootFromObject )
{
    {
        caffa::ObjectHandle* obj = nullptr;
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::findRoot( obj ) );
    }

    {
        auto obj = std::make_unique<SimpleObj>();
        EXPECT_EQ( obj.get(), caffa::ReferenceHelper::findRoot( obj.get() ) );
    }

    {
        auto s1      = std::make_unique<SimpleObj>();
        auto ihd1    = std::make_unique<ReferenceSimpleObj>();
        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );

        EXPECT_EQ( ihd1.get(), caffa::ReferenceHelper::findRoot( raw_s1p ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FindRootFromField )
{
    {
        caffa::FieldHandle* fieldHandle = nullptr;
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::findRoot( fieldHandle ) );
    }

    {
        auto s1      = std::make_unique<SimpleObj>();
        auto ihd1    = std::make_unique<ReferenceSimpleObj>();
        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );

        EXPECT_EQ( ihd1.get(), caffa::ReferenceHelper::findRoot( &raw_s1p->m_dir ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFrommRootToField )
{
    {
        caffa::ObjectHandle* obj         = nullptr;
        caffa::FieldHandle*  fieldHandle = nullptr;
        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToField( obj, fieldHandle ).empty() );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto s3   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );
        auto raw_s3p = ihd1->m_simpleObjPtrField.push_back( std::move( s3 ) );

        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToField( nullptr, &raw_s3p->m_dir ).empty() );
        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToField( ihd1.get(), nullptr ).empty() );

        std::string refString      = caffa::ReferenceHelper::referenceFromRootToField( ihd1.get(), &raw_s3p->m_dir );
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
        caffa::ObjectHandle* root = nullptr;
        caffa::ObjectHandle* obj  = nullptr;
        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToObject( root, obj ).empty() );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto s3   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );
        auto raw_s3p = ihd1->m_simpleObjPtrField.push_back( std::move( s3 ) );

        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToObject( nullptr, raw_s3p ).empty() );
        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToObject( ihd1.get(), nullptr ).empty() );

        std::string refString      = caffa::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s3p );
        std::string expectedString = "m_simpleObjPtrField 2";
        EXPECT_STREQ( expectedString.c_str(), refString.c_str() );

        auto ihd2    = std::make_unique<ReferenceSimpleObj>();
        auto s4      = std::make_unique<SimpleObj>();
        auto raw_s4p = ihd2->m_simpleObjPtrField.push_back( std::move( s4 ) );

        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s4p ).empty() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ObjectFromReference )
{
    {
        caffa::ObjectHandle* root = nullptr;
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( root, "" ) );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( root, "a 2 b 4" ) );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( ihd1.get(), "" ) );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( ihd1.get(), "a 2 b 4" ) );

        std::string refString = caffa::ReferenceHelper::referenceFromRootToObject( ihd1.get(), raw_s2p );
        EXPECT_EQ( raw_s2p, caffa::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );

        std::unique_ptr<caffa::ObjectHandle> removedObject = ihd1->m_simpleObjPtrField.remove( raw_s2p );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );

        ihd1->m_simpleObjPtrField.clear();

        EXPECT_EQ( nullptr, caffa::ReferenceHelper::objectFromReference( ihd1.get(), refString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, FieldFromReference )
{
    {
        caffa::ObjectHandle* root = nullptr;
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( root, "" ) );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( root, "a 2 b 4" ) );
    }

    {
        auto s1   = std::make_unique<SimpleObj>();
        auto s2   = std::make_unique<SimpleObj>();
        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        auto raw_s1p = ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        auto raw_s2p = ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( ihd1.get(), "" ) );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( ihd1.get(), "a 2 b 4" ) );

        caffa::FieldHandle* fHandle = &raw_s2p->m_position;

        std::string refString = caffa::ReferenceHelper::referenceFromRootToField( ihd1.get(), fHandle );
        EXPECT_EQ( fHandle, caffa::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );

        std::unique_ptr<caffa::ObjectHandle> removedObject = ihd1->m_simpleObjPtrField.remove( raw_s2p );
        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );

        ihd1->m_simpleObjPtrField.clear();

        EXPECT_EQ( nullptr, caffa::ReferenceHelper::fieldFromReference( ihd1.get(), refString ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( ReferenceHelperTest, ReferenceFromFieldToObject )
{
    {
        caffa::ObjectHandle* root        = nullptr;
        caffa::FieldHandle*  fieldHandle = nullptr;

        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromFieldToObject( fieldHandle, root ).empty() );
    }

    {
        auto s1 = std::make_unique<SimpleObj>();
        auto s2 = std::make_unique<SimpleObj>();

        caffa::FieldHandle* s2FieldHandle = &s2->m_dir;

        // Unrelated objects
        EXPECT_TRUE( caffa::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, s1.get() ).empty() );

        auto root    = std::make_unique<ReferenceSimpleObj>();
        auto root_s1 = std::make_unique<SimpleObj>();
        root->m_simpleObjPtrField.push_back( std::move( root_s1 ) );

        auto ihd1 = std::make_unique<ReferenceSimpleObj>();

        ihd1->m_simpleObjPtrField.push_back( std::move( s1 ) );
        ihd1->m_simpleObjPtrField.push_back( std::move( s2 ) );

        root->m_pointersField = std::move( ihd1 );

        std::string        refString = caffa::ReferenceHelper::referenceFromFieldToObject( s2FieldHandle, root_s1.get() );
        caffa::ObjectHandle* obj       = caffa::ReferenceHelper::objectFromFieldReference( s2FieldHandle, refString );
        EXPECT_EQ( root_s1.get(), obj );
    }
}
