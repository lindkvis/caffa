
#include "gtest.h"

#include "cafChildArrayField.h"
#include "cafObject.h"
#include "cafUiTreeView.h"

#include <QApplication>
#include <QModelIndex>

using namespace caffa;

class SimpleObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    SimpleObj()
        : Object()
    {
        assignUiInfo( "SimpleObj", "", "Tooltip SimpleObj", "WhatsThis SimpleObj" );
    }
};
CAFFA_SOURCE_INIT( SimpleObj, "SimpleObj", "Object" );

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    DemoObject()
    {
        assignUiInfo( "DemoObject", "", "Tooltip DemoObject", "WhatsThis DemoObject" );

        initField( m_simpleObjPtrField, "SimpleObjPtrField" ).withUi( "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
    }

    caffa::ChildArrayField<caffa::ObjectHandle*> m_simpleObjPtrField;
};

CAFFA_SOURCE_INIT( DemoObject, "DemoObject", "Object" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, DeleteOneItemAndVerifyTreeOrdering )
{
    auto obj1 = std::make_unique<SimpleObj>();
    auto obj2 = std::make_unique<SimpleObj>();
    auto obj3 = std::make_unique<SimpleObj>();
    auto obj4 = std::make_unique<SimpleObj>();

    auto demoObj = std::make_unique<DemoObject>();

    auto obj1p = obj1.get();

    demoObj->m_simpleObjPtrField.push_back( std::move( obj1 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj2 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj3 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj4 ) );

    UiTreeView treeView;
    treeView.setItem( demoObj.get() );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj1p );
    EXPECT_TRUE( mi.isValid() );

    auto newobj1 = demoObj->m_simpleObjPtrField.remove( obj1p );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj1p );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, AddOneItemAndVerifyTreeOrdering )
{
    auto obj1 = std::make_unique<SimpleObj>();
    auto obj2 = std::make_unique<SimpleObj>();
    auto obj3 = std::make_unique<SimpleObj>();
    auto obj4 = std::make_unique<SimpleObj>();

    auto demoObj = std::make_unique<DemoObject>();
    demoObj->m_simpleObjPtrField.push_back( std::move( obj1 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj2 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj3 ) );

    UiTreeView treeView;
    treeView.setItem( demoObj.get() );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4.get() );
    EXPECT_FALSE( mi.isValid() );

    auto obj4p = obj4.get();

    demoObj->m_simpleObjPtrField.push_back( std::move( obj4 ) );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4p );
    EXPECT_TRUE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, ChangeOrderingAndVerifyTreeOrdering )
{
    auto obj1 = std::make_unique<SimpleObj>();
    auto obj2 = std::make_unique<SimpleObj>();
    auto obj3 = std::make_unique<SimpleObj>();
    auto obj4 = std::make_unique<SimpleObj>();

    auto obj4p = obj4.get();

    auto demoObj = std::make_unique<DemoObject>();
    demoObj->m_simpleObjPtrField.push_back( std::move( obj1 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj2 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj3 ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( obj4 ) );

    UiTreeView treeView;
    treeView.setItem( demoObj.get() );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4p );
    EXPECT_EQ( 3, mi.row() );

    auto detachedObjects = demoObj->m_simpleObjPtrField.removeAll();
    demoObj->m_simpleObjPtrField.push_back( std::move( detachedObjects[0] ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( detachedObjects[3] ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( detachedObjects[2] ) );
    demoObj->m_simpleObjPtrField.push_back( std::move( detachedObjects[1] ) );

    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4p );
    EXPECT_EQ( 1, mi.row() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, ChangeDeepInTreeNotifyRootAndVerifyTreeOrdering )
{
    auto root = std::make_unique<DemoObject>();

    auto rootObj1 = std::make_unique<SimpleObj>();
    root->m_simpleObjPtrField.push_back( std::move( rootObj1 ) );

    auto demoObj  = std::make_unique<DemoObject>();
    auto demoObjp = demoObj.get();
    root->m_simpleObjPtrField.push_back( std::move( demoObj ) );

    auto obj1 = std::make_unique<SimpleObj>();
    auto obj2 = std::make_unique<SimpleObj>();
    auto obj3 = std::make_unique<SimpleObj>();
    auto obj4 = std::make_unique<SimpleObj>();

    auto obj4p = obj4.get();

    demoObjp->m_simpleObjPtrField.push_back( std::move( obj1 ) );
    demoObjp->m_simpleObjPtrField.push_back( std::move( obj2 ) );
    demoObjp->m_simpleObjPtrField.push_back( std::move( obj3 ) );
    demoObjp->m_simpleObjPtrField.push_back( std::move( obj4 ) );

    UiTreeView treeView;
    treeView.setItem( root.get() );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4p );
    EXPECT_EQ( 3, mi.row() );

    auto new_obj4 = demoObjp->m_simpleObjPtrField.remove( obj4p );

    root->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4p );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, DISABLED_PerformanceLargeNumberOfItems )
{
    // int objCount = 20000;
    int objCount = 100000;

    auto demoObj = std::make_unique<DemoObject>();
    for ( int i = 0; i < objCount; i++ )
    {
        demoObj->m_simpleObjPtrField.push_back( std::make_unique<SimpleObj>() );
    }

    UiTreeView treeView;
    treeView.setItem( demoObj.get() );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();
}
