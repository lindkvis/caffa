
#include "gtest/gtest.h"

#include "cafChildArrayField.h"
#include "cafObject.h"
#include "cafUiTreeView.h"

#include <QApplication>
#include <QModelIndex>

using namespace caf;

class SimpleObj : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SimpleObj()
        : Object()
    {
        initObject().withUi( "SimpleObj", "", "Tooltip SimpleObj", "WhatsThis SimpleObj" );
    }
};
CAF_SOURCE_INIT( SimpleObj, "SimpleObj" );

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        initObject().withUi( "DemoObject", "", "Tooltip DemoObject", "WhatsThis DemoObject" );

        initField( m_simpleObjPtrField, "SimpleObjPtrField" ).withUi( "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
    }

    caf::ChildArrayField<caf::ObjectHandle*> m_simpleObjPtrField;
};

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, DeleteOneItemAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoObject* demoObj = new DemoObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    UiTreeView treeView;
    treeView.setItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj1 );
    EXPECT_TRUE( mi.isValid() );

    demoObj->m_simpleObjPtrField.removeChildObject( obj1 );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj1 );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, AddOneItemAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoObject* demoObj = new DemoObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );

    UiTreeView treeView;
    treeView.setItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_FALSE( mi.isValid() );

    demoObj->m_simpleObjPtrField.push_back( obj4 );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_TRUE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, ChangeOrderingAndVerifyTreeOrdering )
{
    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;

    DemoObject* demoObj = new DemoObject;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    UiTreeView treeView;
    treeView.setItem( demoObj );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 3, mi.row() );

    demoObj->m_simpleObjPtrField.clear();
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );

    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 1, mi.row() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, ChangeDeepInTreeNotifyRootAndVerifyTreeOrdering )
{
    DemoObject* root = new DemoObject;

    SimpleObj* rootObj1 = new SimpleObj;
    root->m_simpleObjPtrField.push_back( rootObj1 );

    DemoObject* demoObj = new DemoObject;
    root->m_simpleObjPtrField.push_back( demoObj );

    SimpleObj* obj1 = new SimpleObj;
    SimpleObj* obj2 = new SimpleObj;
    SimpleObj* obj3 = new SimpleObj;
    SimpleObj* obj4 = new SimpleObj;
    demoObj->m_simpleObjPtrField.push_back( obj1 );
    demoObj->m_simpleObjPtrField.push_back( obj2 );
    demoObj->m_simpleObjPtrField.push_back( obj3 );
    demoObj->m_simpleObjPtrField.push_back( obj4 );

    UiTreeView treeView;
    treeView.setItem( root );

    QModelIndex mi;
    mi = treeView.findModelIndex( obj4 );
    EXPECT_EQ( 3, mi.row() );

    demoObj->m_simpleObjPtrField.removeChildObject( obj4 );

    root->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();

    mi = treeView.findModelIndex( obj4 );
    EXPECT_FALSE( mi.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeViewModelTest, DISABLED_PerformanceLargeNumberOfItems )
{
    // int objCount = 20000;
    int objCount = 100000;

    DemoObject* demoObj = new DemoObject;
    for ( int i = 0; i < objCount; i++ )
    {
        demoObj->m_simpleObjPtrField.push_back( new SimpleObj );
    }

    UiTreeView treeView;
    treeView.setItem( demoObj );
    demoObj->m_simpleObjPtrField().capability<FieldUiCapability>()->updateConnectedEditors();
}
