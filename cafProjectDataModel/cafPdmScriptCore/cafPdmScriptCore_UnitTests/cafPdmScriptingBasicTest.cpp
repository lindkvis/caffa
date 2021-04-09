//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafPdmPointer.h"
#include "cafPdmReferenceHelper.h"

#include <memory>

/// Demo objects to show the usage of the Pdm system

class SimpleObj : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SimpleObj()
        : Object()
        , m_doubleMember( 0.0 )
    {
        initObject( "SimpleObj", "", "Tooltip SimpleObj", "WhatsThis SimpleObj" );

        CAF_InitField( &m_position, "Position", 8765.2, "Position", "", "Tooltip", "WhatsThis" );
        CAF_InitField( &m_dir, "Dir", 123.56, "Direction", "", "Tooltip", "WhatsThis" );
        CAF_InitField( &m_up, "Up", 0.0, "Up value", "", "Tooltip", "WhatsThis" );
        CAF_InitFieldNoDefault( &m_numbers, "Numbers", "Important Numbers", "", "Tooltip", "WhatsThis" );
#if 1
        auto doubleProxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        doubleProxyAccessor->registerSetMethod( this, &SimpleObj::setDoubleMember );
        doubleProxyAccessor->registerGetMethod( this, &SimpleObj::doubleMember );
        m_proxyDouble.setFieldDataAccessor( std::move( doubleProxyAccessor ) );
        AddUiCapabilityToField( &m_proxyDouble );
        AddIoCapabilityToField( &m_proxyDouble );
        CAF_InitFieldNoDefault( &m_proxyDouble, "ProxyDouble", "ProxyDouble", "", "", "" );
#endif
    }

    /// Assignment and copying of PDM objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj( const SimpleObj& other )
        : Object()
    {
        CAF_InitField( &m_position, "Position", 8765.2, "Position", "", "", "WhatsThis" );
        CAF_InitField( &m_dir, "Dir", 123.56, "Direction", "", "", "WhatsThis" );
        CAF_InitField( &m_up, "Up", 0.0, "Up value", "", "", "WhatsThis" );

        CAF_InitFieldNoDefault( &m_numbers, "Numbers", "Important Numbers", "", "", "WhatsThis" );

        m_position     = other.m_position;
        m_dir          = other.m_dir;
        m_up           = other.m_up;
        m_numbers      = other.m_numbers;
        m_doubleMember = other.m_doubleMember;
    }

    ~SimpleObj() {}

    caf::Field<double>              m_position;
    caf::Field<double>              m_dir;
    caf::Field<double>              m_up;
    caf::Field<std::vector<double>> m_numbers;
    caf::DataValueField<double>     m_proxyDouble;

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
CAF_SOURCE_INIT( SimpleObj, "SimpleObj" );

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        initObject( "DemoObject", "", "Tooltip DemoObject", "WhatsThis DemoObject" );

        CAF_InitField( &m_doubleField,
                       "BigNumber",
                       0.0,
                       "",
                       "",
                       "Enter a big number here",
                       "This is a place you can enter a big real value if you want" );

        CAF_InitField( &m_intField,
                       "IntNumber",
                       0,
                       "",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );

        CAF_InitField( &m_textField, "TextField", std::string( "??? Test text   end" ), "TextField", "", "Tooltip", "WhatsThis" );
        CAF_InitFieldNoDefault( &m_simpleObjPtrField, "SimpleObjPtrField", "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
        CAF_InitFieldNoDefault( &m_simpleObjPtrField2, "SimpleObjPtrField2", "SimpleObjPtrField2", "", "Tooltip", "WhatsThis" );
        m_simpleObjPtrField2 = new SimpleObj;
    }

    ~DemoObject()
    {
        delete m_simpleObjPtrField();
        delete m_simpleObjPtrField2();
    }

    // Fields
    caf::Field<double>      m_doubleField;
    caf::Field<int>         m_intField;
    caf::Field<std::string> m_textField;

    caf::ChildField<SimpleObj*> m_simpleObjPtrField;
    caf::ChildField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_SOURCE_INIT( DemoObject, "DemoObject" );

class InheritedDemoObj : public DemoObject
{
    CAF_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    InheritedDemoObj()
    {
        initObject( "InheritedDemoObj", "", "ToolTip InheritedDemoObj", "Whatsthis InheritedDemoObj" );

        CAF_InitScriptableFieldNoDefault( &m_texts, "Texts", "Some words", "", "", "" );
        CAF_InitScriptableFieldNoDefault( &m_numbers, "Numbers", "Some words", "", "", "" );
        CAF_InitFieldNoDefault( &m_testEnumField, "TestEnumValue", "An Enum", "", "", "" );
        CAF_InitFieldNoDefault( &m_simpleObjectsField,
                                "SimpleObjects",
                                "SimpleObjectsField",
                                "",
                                "ToolTip SimpleObjectsField",
                                "Whatsthis SimpleObjectsField" );
    }

    ~InheritedDemoObj() { m_simpleObjectsField.deleteAllChildObjects(); }

    caf::Field<std::vector<std::string>> m_texts;
    caf::Field<std::vector<double>>      m_numbers;

    caf::Field<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::ChildArrayField<SimpleObj*>       m_simpleObjectsField;
};
CAF_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj" );

namespace caf
{
template <>
void AppEnum<InheritedDemoObj::TestEnumType>::setUp()
{
    addItem( InheritedDemoObj::T1, "T1", "An A letter" );
    addItem( InheritedDemoObj::T2, "T2", "A B letter" );
    addItem( InheritedDemoObj::T3, "T3", "A B letter" );
    setDefault( InheritedDemoObj::T1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/* TEST( PdmScriptingTest, BasicUse )
{
    std::unique_ptr<MyPdmDocument> document( new MyPdmDocument );

    std::string fileExt = "py";

    std::unique_ptr<caf::PdmCodeGenerator> generator( caf::PdmCodeGeneratorFactory::instance()->create( fileExt ) );
    auto                                   generatedText = generator->generate( caf::DefaultObjectFactory::instance() );

    auto string = generatedText.toStdString();
}*/

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmScriptingTest, CheckIsVector )
{
    InheritedDemoObj obj;

    auto isVector = obj.m_numbers.capability<caf::FieldIoCapability>()->isVectorField();
    EXPECT_TRUE( isVector );

    // auto xmlCap = obj.xmlCapability();
    // auto string = xmlCap->writeObjectToXmlString();
}
