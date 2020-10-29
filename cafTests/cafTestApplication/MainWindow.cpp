
#include "cafField.h"

#include "MainWindow.h"

#include "CustomObjectEditor.h"
#include "ManyGroups.h"
#include "MenuItemProducer.h"
#include "TamComboBox.h"
#include "WidgetLayoutTest.h"

#include "cafAppEnum.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdSelectionHelper.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafFilePath.h"
#include "cafPdmDocument.h"
#include "cafFieldUiCapability.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafPdmProxyValueField.h"
#include "cafPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableView.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafPdmUiTreeView.h"
#include "cafQActionWrapper.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QUndoView>

class DemoObjectGroup : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    DemoObjectGroup()
    {
        CAF_PDM_InitFieldNoDefault(&objects, "Objects", "", "", "", "")

            objects.capability<caf::FieldUiCapability>()
                ->setUiHidden(true);
    }

public:
    caf::ChildArrayField<ObjectHandle*> objects;
};

CAF_PDM_SOURCE_INIT(DemoObjectGroup, "DemoObjectGroup");

class TinyDemoObject : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    TinyDemoObject();

private:
    caf::Field<bool>   m_toggleField;
    caf::Field<double> m_doubleField;
};

CAF_PDM_SOURCE_INIT(TinyDemoObject, "TinyDemoObject");

TinyDemoObject::TinyDemoObject()
{
    CAF_PDM_InitObject("Tiny Demo Object", "", "This object is a demo of the CAF framework", "");
    CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Item", "", "Tooltip", " Whatsthis?");
    CAF_PDM_InitField(&m_doubleField,
                      "Number",
                      0.0,
                      "Number",
                      "",
                      "Enter a floating point number here",
                      "Double precision floating point number");
}

class SmallDemoObject : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    SmallDemoObject()
    {
        CAF_PDM_InitObject("Small Demo Object",
                           ":/images/win/filenew.png",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(
            &m_toggleField, "Toggle", false, "Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField,
                          "BigNumber",
                          0.0,
                          "Big Number",
                          "",
                          "Enter a big number here",
                          "This is a place you can enter a big real value if you want");
        m_doubleField.capability<caf::FieldUiCapability>()->setCustomContextMenuEnabled(true);

        CAF_PDM_InitField(&m_intField,
                          "IntNumber",
                          0,
                          "Small Number",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField,
                          "TextField",
                          QString(""),
                          "Text",
                          "",
                          "Text tooltip",
                          "This is a place you can enter a small integer value if you want");

        m_proxyDoubleField.registerSetMethod(this, &SmallDemoObject::setDoubleMember);
        m_proxyDoubleField.registerGetMethod(this, &SmallDemoObject::doubleMember);
        CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

        CAF_PDM_InitField(&m_fileName, "FileName", caf::FilePath("filename"), "File Name", "", "", "");

        CAF_PDM_InitFieldNoDefault(&m_fileNameList, "FileNameList", "File Name List", "", "", "");
        m_fileNameList.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

        m_proxyDoubleField = 0;
        if (!(m_proxyDoubleField == 3))
        {
            qDebug() << "Double is not 3 ";
        }

        CAF_PDM_InitFieldNoDefault(&m_multiSelectList, "SelectedItems", "Multi Select Field", "", "", "");
        m_multiSelectList.capability<caf::FieldIoCapability>()->setIOReadable(false);
        m_multiSelectList.capability<caf::FieldIoCapability>()->setIOWritable(false);
        m_multiSelectList.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

        m_multiSelectList.v().push_back("First");
        m_multiSelectList.v().push_back("Second");
        m_multiSelectList.v().push_back("Third");
    }

    caf::Field<double>  m_doubleField;
    caf::Field<int>     m_intField;
    caf::Field<QString> m_textField;

    caf::PdmProxyValueField<double>           m_proxyDoubleField;
    caf::Field<caf::FilePath>              m_fileName;
    caf::Field<std::vector<caf::FilePath>> m_fileNameList;

    caf::Field<std::vector<QString>> m_multiSelectList;

    caf::Field<bool>  m_toggleField;
    caf::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if (changedField == &m_toggleField)
        {
            qDebug() << "Toggle Field changed";
        }
    }

    void setDoubleMember(const double& d)
    {
        m_doubleMember = d;
        qDebug() << "setDoubleMember";
    }
    double doubleMember() const
    {
        qDebug() << "doubleMember";
        return m_doubleMember;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override
    {
        QList<caf::PdmOptionItemInfo> options;

        if (fieldNeedingOptions == &m_multiSelectList)
        {
            QString text;

            text = "First";
            options.push_back(caf::PdmOptionItemInfo(text, text));

            text = "Second";
            options.push_back(caf::PdmOptionItemInfo::createHeader(text, false, caf::IconProvider(":/images/win/textbold.png")));

            {
                text                            = "Second_a";
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text, true);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            {
                text = "Second_b";
                caf::PdmOptionItemInfo itemInfo =
                    caf::PdmOptionItemInfo(text, text, false, caf::IconProvider(":/images/win/filenew.png"));
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            int additionalSubItems = 2;
            for (auto i = 0; i < additionalSubItems; i++)
            {
                text                            = "Second_b_" + QString::number(i);
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            static int s_additionalSubItems = 0;
            if (m_toggleField())
            {
                s_additionalSubItems++;
            }
            for (auto i = 0; i < s_additionalSubItems; i++)
            {
                text                            = "Second_b_" + QString::number(i);
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            text = "Third";
            options.push_back(caf::PdmOptionItemInfo(text, text));

            text = "Fourth";
            options.push_back(caf::PdmOptionItemInfo(text, text));
        }

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineCustomContextMenu(const caf::FieldHandle* fieldNeedingMenu,
                                 caf::MenuInterface*        menu,
                                 QWidget*                   fieldEditorWidget) override
    {
        menu->addAction(caf::CmdFeatureManager::instance()->action("test", "nothing"));
        menu->addAction(caf::CmdFeatureManager::instance()->action("other test <<>>", "still nothing"));
    }

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_doubleField);
        uiOrdering.add(&m_intField);
        QString dynamicGroupName = QString("Dynamic Group Text (%1)").arg(m_intField);

        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword(dynamicGroupName, "MyTest");
        group->add(&m_textField);
        group->add(&m_proxyDoubleField);
    }
};

CAF_PDM_SOURCE_INIT(SmallDemoObject, "SmallDemoObject");

class SmallGridDemoObject : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    SmallGridDemoObject()
    {
        CAF_PDM_InitObject("Small Grid Demo Object",
                           "",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_intFieldStandard,
                          "Standard",
                          0,
                          "Standard",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldUseFullSpace,
                          "FullSpace",
                          0,
                          "Use Full Space For Both",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldUseFullSpaceLabel,
                          "FullSpaceLabel",
                          0,
                          "Total 3, Label MAX",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldUseFullSpaceField,
                          "FullSpaceField",
                          0,
                          "Total MAX, Label 1",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldWideLabel,
                          "WideLabel",
                          0,
                          "Wide Label",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldWideField,
                          "WideField",
                          0,
                          "Wide Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldLeft,
                          "LeftField",
                          0,
                          "Left Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldRight,
                          "RightField",
                          0,
                          "Right Field With More Text",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldWideBoth,
                          "WideBoth",
                          0,
                          "Wide Both",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");

        CAF_PDM_InitField(&m_intFieldWideBoth2,
                          "WideBoth2",
                          0,
                          "Wide Both",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldLeft2,
                          "LeftFieldInGrp",
                          0,
                          "Left Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldCenter,
                          "CenterFieldInGrp",
                          0,
                          "Center Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldRight2,
                          "RightFieldInGrp",
                          0,
                          "Right Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldLabelTop,
                          "FieldLabelTop",
                          0,
                          "Field Label Top",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        m_intFieldLabelTop.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        CAF_PDM_InitField(&m_stringFieldLabelHidden,
                          "FieldLabelHidden",
                          QString("Hidden Label Field"),
                          "Field Label Hidden",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHidden.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

        CAF_PDM_InitField(&m_intFieldWideBothAuto,
                          "WideBothAuto",
                          0,
                          "Wide ",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldLeftAuto,
                          "LeftFieldInGrpAuto",
                          0,
                          "Left Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldCenterAuto,
                          "CenterFieldInGrpAuto",
                          0,
                          "Center Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldRightAuto,
                          "RightFieldInGrpAuto",
                          0,
                          "Right Field",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldLabelTopAuto,
                          "FieldLabelTopAuto",
                          0,
                          "Field Label Top",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        m_intFieldLabelTopAuto.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        CAF_PDM_InitField(&m_stringFieldLabelHiddenAuto,
                          "FieldLabelHiddenAuto",
                          QString("Hidden Label Field"),
                          "Field Label Hidden",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHiddenAuto.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

        CAF_PDM_InitField(&m_intFieldLeftOfGroup,
                          "FieldLeftOfGrp",
                          0,
                          "Left of group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldRightOfGroup,
                          "FieldRightOfGrp",
                          0,
                          "Right of group wide label",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");

        CAF_PDM_InitField(&m_intFieldInsideGroup1,
                          "FieldInGrp1",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldInsideGroup2,
                          "FieldInGrp2",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldInsideGroup3,
                          "FieldInGrp3",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldInsideGroup4,
                          "FieldInGrp4",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldInsideGroup5,
                          "FieldInGrp5",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_intFieldInsideGroup6,
                          "FieldInGrp6",
                          0,
                          "Inside Group",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
    }

    // Outside group
    caf::Field<int> m_intFieldStandard;
    caf::Field<int> m_intFieldUseFullSpace;
    caf::Field<int> m_intFieldUseFullSpaceLabel;
    caf::Field<int> m_intFieldUseFullSpaceField;
    caf::Field<int> m_intFieldWideLabel;
    caf::Field<int> m_intFieldWideField;
    caf::Field<int> m_intFieldWideBoth;
    caf::Field<int> m_intFieldLeft;
    caf::Field<int> m_intFieldRight;

    // First group
    caf::Field<int>     m_intFieldWideBoth2;
    caf::Field<int>     m_intFieldLeft2;
    caf::Field<int>     m_intFieldCenter;
    caf::Field<int>     m_intFieldRight2;
    caf::Field<int>     m_intFieldLabelTop;
    caf::Field<QString> m_stringFieldLabelHidden;

    // Auto group
    caf::Field<int>     m_intFieldWideBothAuto;
    caf::Field<int>     m_intFieldLeftAuto;
    caf::Field<int>     m_intFieldCenterAuto;
    caf::Field<int>     m_intFieldRightAuto;
    caf::Field<int>     m_intFieldLabelTopAuto;
    caf::Field<QString> m_stringFieldLabelHiddenAuto;

    // Combination with groups
    caf::Field<int> m_intFieldLeftOfGroup;
    caf::Field<int> m_intFieldRightOfGroup;
    caf::Field<int> m_intFieldInsideGroup1;
    caf::Field<int> m_intFieldInsideGroup2;

    // Side-by-side groups
    caf::Field<int> m_intFieldInsideGroup3;
    caf::Field<int> m_intFieldInsideGroup4;
    caf::Field<int> m_intFieldInsideGroup5;
    caf::Field<int> m_intFieldInsideGroup6;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
        uiOrdering.add(&m_intFieldUseFullSpace,
                       caf::PdmUiOrdering::LayoutOptions(true,
                                                         caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN,
                                                         caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceLabel,
                       caf::PdmUiOrdering::LayoutOptions(true, 3, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceField,
                       caf::PdmUiOrdering::LayoutOptions(true, caf::PdmUiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1));
        uiOrdering.add(&m_intFieldWideLabel, caf::PdmUiOrdering::LayoutOptions(true, 4, 3));
        uiOrdering.add(&m_intFieldWideField, caf::PdmUiOrdering::LayoutOptions(true, 4, 1));
        uiOrdering.add(&m_intFieldLeft, caf::PdmUiOrdering::LayoutOptions(true));
        uiOrdering.add(&m_intFieldRight, caf::PdmUiOrdering::LayoutOptions(false));
        uiOrdering.add(&m_intFieldWideBoth, caf::PdmUiOrdering::LayoutOptions(true, 4, 2));

        QString dynamicGroupName = QString("Dynamic Group Text (%1)").arg(m_intFieldStandard);

        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Wide Group", {true, 4});
        group->add(&m_intFieldWideBoth2, caf::PdmUiOrdering::LayoutOptions(true, 6, 3));
        group->add(&m_intFieldLeft2, caf::PdmUiOrdering::LayoutOptions(true));
        group->add(&m_intFieldCenter, caf::PdmUiOrdering::LayoutOptions(false));
        group->add(&m_intFieldRight2, caf::PdmUiOrdering::LayoutOptions(false));
        group->add(&m_intFieldLabelTop, caf::PdmUiOrdering::LayoutOptions(true, 6));
        group->add(&m_stringFieldLabelHidden, caf::PdmUiOrdering::LayoutOptions(true, 6));

        caf::PdmUiGroup* autoGroup =
            uiOrdering.addNewGroup("Automatic Full Width Group", caf::PdmUiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldWideBothAuto, caf::PdmUiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldLeftAuto, caf::PdmUiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldCenterAuto, false);
        autoGroup->add(&m_intFieldRightAuto, caf::PdmUiOrdering::LayoutOptions(false));
        autoGroup->add(&m_intFieldLabelTopAuto, true);
        autoGroup->add(&m_stringFieldLabelHiddenAuto, true);

        uiOrdering.add(&m_intFieldLeftOfGroup);
        caf::PdmUiGroup* group2 = uiOrdering.addNewGroup("Right Group", caf::PdmUiOrdering::LayoutOptions(false, 2, 0));
        group2->setEnableFrame(false);
        group2->add(&m_intFieldInsideGroup1);

        caf::PdmUiGroup* group3 = uiOrdering.addNewGroup("Narrow L", caf::PdmUiOrdering::LayoutOptions(true, 1));
        group3->add(&m_intFieldInsideGroup2);
        uiOrdering.add(&m_intFieldRightOfGroup, caf::PdmUiOrdering::LayoutOptions(false, 3, 2));

        caf::PdmUiGroup* groupL = uiOrdering.addNewGroup("Left Group", caf::PdmUiOrdering::LayoutOptions(true, 1));
        groupL->add(&m_intFieldInsideGroup3);
        groupL->add(&m_intFieldInsideGroup5);
        caf::PdmUiGroup* groupR = uiOrdering.addNewGroup("Right Wide Group", caf::PdmUiOrdering::LayoutOptions(false, 3));
        groupR->setEnableFrame(false);
        groupR->add(&m_intFieldInsideGroup4);
        groupR->add(&m_intFieldInsideGroup6);
    }
};

CAF_PDM_SOURCE_INIT(SmallGridDemoObject, "SmallGridDemoObject");

class SingleEditorObject : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    SingleEditorObject()
    {
        CAF_PDM_InitObject("Single Editor Object",
                           "",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_intFieldStandard,
                          "Standard",
                          0,
                          "Fairly Wide Label",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
    }

    // Outside group
    caf::Field<int> m_intFieldStandard;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
    }
};

CAF_PDM_SOURCE_INIT(SingleEditorObject, "SingleEditorObject");

class SmallDemoObjectA : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    SmallDemoObjectA()
    {
        CAF_PDM_InitObject("Small Demo Object A",
                           "",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_pushButtonField, "Push", false, "Button Field", "", "", " ");
        CAF_PDM_InitField(&m_doubleField,
                          "BigNumber",
                          0.0,
                          "Big Number",
                          "",
                          "Enter a big number here",
                          "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField,
                          "IntNumber",
                          0,
                          "Small Number",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString("Small Demo Object A"), "Name Text Field", "", "", "");
        CAF_PDM_InitField(&m_testEnumField, "TestEnumValue", caf::AppEnum<TestEnumType>(T1), "EnumField", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_ptrField, "m_ptrField", "PtrField", "", "", "");

        CAF_PDM_InitFieldNoDefault(&m_proxyEnumField, "ProxyEnumValue", "ProxyEnum", "", "", "");
        m_proxyEnumField.registerSetMethod(this, &SmallDemoObjectA::setEnumMember);
        m_proxyEnumField.registerGetMethod(this, &SmallDemoObjectA::enumMember);
        m_proxyEnumMember = T2;

        m_testEnumField.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

        CAF_PDM_InitFieldNoDefault(&m_multipleAppEnum, "MultipleAppEnumValue", "MultipleAppEnumValue", "", "", "");
        m_multipleAppEnum.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
        CAF_PDM_InitFieldNoDefault(&m_highlightedEnum, "HighlightedEnum", "HighlightedEnum", "", "", "");
        m_highlightedEnum.capability<caf::FieldUiCapability>()->setUiHidden(true);
    }

    caf::Field<double>                     m_doubleField;
    caf::Field<int>                        m_intField;
    caf::Field<QString>                    m_textField;
    caf::Field<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::PtrField<SmallDemoObjectA*>    m_ptrField;

    caf::PdmProxyValueField<caf::AppEnum<TestEnumType>> m_proxyEnumField;
    void                                                setEnumMember(const caf::AppEnum<TestEnumType>& val)
    {
        m_proxyEnumMember = val;
    }
    caf::AppEnum<TestEnumType> enumMember() const
    {
        return m_proxyEnumMember;
    }
    TestEnumType m_proxyEnumMember;

    // vector of app enum
    caf::Field<std::vector<caf::AppEnum<TestEnumType>>> m_multipleAppEnum;
    caf::Field<caf::AppEnum<TestEnumType>>              m_highlightedEnum;

    caf::Field<bool> m_toggleField;
    caf::Field<bool> m_pushButtonField;

    caf::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if (changedField == &m_toggleField)
        {
            qDebug() << "Toggle Field changed";
        }
        else if (changedField == &m_highlightedEnum)
        {
            qDebug() << "Highlight value " << m_highlightedEnum();
        }
        else if (changedField == &m_pushButtonField)
        {
            qDebug() << "Push Button pressed ";
        }
    }

    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override
    {
        QList<caf::PdmOptionItemInfo> options;

        if (&m_ptrField == fieldNeedingOptions)
        {
            caf::FieldHandle*               field;
            std::vector<caf::ObjectHandle*> objects;
            field = this->parentField();

            field->childObjects(&objects);

            for (size_t i = 0; i < objects.size(); ++i)
            {
                QString userDesc;

                caf::ObjectUiCapability* uiObject = caf::uiObj(objects[i]);
                if (uiObject)
                {
                    if (uiObject->userDescriptionField())
                    {
                        caf::FieldUiCapability* uiFieldHandle =
                            uiObject->userDescriptionField()->capability<caf::FieldUiCapability>();
                        if (uiFieldHandle)
                        {
                            userDesc = uiFieldHandle->uiValue().toString();
                        }
                    }

                    options.push_back(
                        caf::PdmOptionItemInfo(uiObject->uiName() + "(" + userDesc + ")",
                                               QVariant::fromValue(caf::PdmPointer<caf::ObjectHandle>(objects[i]))));
                }
            }
        }
        else if (&m_multipleAppEnum == fieldNeedingOptions)
        {
            for (size_t i = 0; i < caf::AppEnum<TestEnumType>::size(); ++i)
            {
                options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<TestEnumType>::uiTextFromIndex(i),
                                                         caf::AppEnum<TestEnumType>::fromIndex(i)));
            }
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caf::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineEditorAttribute(const caf::FieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override
    {
        if (field == &m_multipleAppEnum)
        {
            caf::PdmUiTreeSelectionEditorAttribute* attr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>(attribute);
            if (attr)
            {
                attr->currentIndexFieldHandle = &m_highlightedEnum;
            }
        }
        else if (field == &m_proxyEnumField)
        {
            caf::PdmUiComboBoxEditorAttribute* attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
            if (attr)
            {
                attr->showPreviousAndNextButtons = true;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override
    {
        caf::PdmUiTableViewPushButtonEditorAttribute* attr =
            dynamic_cast<caf::PdmUiTableViewPushButtonEditorAttribute*>(attribute);
        if (attr)
        {
            attr->registerPushButtonTextForFieldKeyword(m_pushButtonField.keyword(), "Edit");
        }
    }
};

CAF_PDM_SOURCE_INIT(SmallDemoObjectA, "SmallDemoObjectA");

namespace caf
{
template<>
void AppEnum<SmallDemoObjectA::TestEnumType>::setUp()
{
    addItem(SmallDemoObjectA::T1, "T1", "An A letter");
    addItem(SmallDemoObjectA::T2, "T2", "A B letter");
    addItem(SmallDemoObjectA::T3, "T3", "A B C letter");
    setDefault(SmallDemoObjectA::T1);
}

} // namespace caf
Q_DECLARE_METATYPE(caf::AppEnum<SmallDemoObjectA::TestEnumType>);

class DemoObject : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    DemoObject()
    {
        CAF_PDM_InitObject(
            "Demo Object", "", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField,
                          "BigNumber",
                          0.0,
                          "Big Number",
                          "",
                          "Enter a big number here",
                          "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField,
                          "IntNumber",
                          0,
                          "Small Number",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_boolField,
                          "BooleanValue",
                          false,
                          "Boolean:",
                          "",
                          "Boolean:Enter some small number here",
                          "Boolean:This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString("Demo Object Description Field"), "Description Field", "", "", "");
        CAF_PDM_InitField(&m_filePath, "FilePath", QString(""), "Filename", "", "", "");
        CAF_PDM_InitField(&m_longText, "LongText", QString("Test text"), "Long Text", "", "", "");

        CAF_PDM_InitFieldNoDefault(
            &m_multiSelectList, "MultiSelect", "Selection List", "", "List", "This is a multi selection list");
        CAF_PDM_InitFieldNoDefault(&m_objectList, "ObjectList", "Objects list Field", "", "List", "This is a list of Objects");
        CAF_PDM_InitFieldNoDefault(&m_objectListOfSameType,
                                   "m_objectListOfSameType",
                                   "Same type Objects list Field",
                                   "",
                                   "Same type List",
                                   "Same type list of Objects");
        m_objectListOfSameType.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::PdmUiTableViewEditor::uiEditorTypeName());
        m_objectListOfSameType.capability<caf::FieldUiCapability>()->setCustomContextMenuEnabled(true);
        ;
        CAF_PDM_InitFieldNoDefault(&m_ptrField, "m_ptrField", "PtrField", "", "Same type List", "Same type list of Objects");

        m_filePath.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
        m_filePath.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        m_longText.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
        m_longText.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

        m_menuItemProducer = new MenuItemProducer;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_objectListOfSameType);
        uiOrdering.add(&m_ptrField);
        uiOrdering.add(&m_boolField);
        caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Name1");
        group1->add(&m_doubleField);
        caf::PdmUiGroup* group2 = uiOrdering.addNewGroup("Name2");
        group2->add(&m_intField);
        caf::PdmUiGroup* group3 = group2->addNewGroup("Name3");
        // group3->add(&m_textField);

        uiOrdering.skipRemainingFields();
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override
    {
        QList<caf::PdmOptionItemInfo> options;
        if (&m_multiSelectList == fieldNeedingOptions)
        {
            options.push_back(caf::PdmOptionItemInfo("Choice 1", "Choice1"));
            options.push_back(caf::PdmOptionItemInfo("Choice 2", "Choice2"));
            options.push_back(caf::PdmOptionItemInfo("Choice 3", "Choice3"));
            options.push_back(caf::PdmOptionItemInfo("Choice 4", "Choice4"));
            options.push_back(caf::PdmOptionItemInfo("Choice 5", "Choice5"));
            options.push_back(caf::PdmOptionItemInfo("Choice 6", "Choice6"));
        }

        if (&m_ptrField == fieldNeedingOptions)
        {
            for (size_t i = 0; i < m_objectListOfSameType.size(); ++i)
            {
                caf::ObjectUiCapability* uiObject = caf::uiObj(m_objectListOfSameType[i]);
                if (uiObject)
                {
                    options.push_back(caf::PdmOptionItemInfo(
                        uiObject->uiName(),
                        QVariant::fromValue(caf::PdmPointer<caf::ObjectHandle>(m_objectListOfSameType[i]))));
                }
            }
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caf::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

    // Fields
    caf::Field<bool>    m_boolField;
    caf::Field<double>  m_doubleField;
    caf::Field<int>     m_intField;
    caf::Field<QString> m_textField;

    caf::Field<QString> m_filePath;

    caf::Field<QString>              m_longText;
    caf::Field<std::vector<QString>> m_multiSelectList;

    caf::ChildArrayField<caf::ObjectHandle*> m_objectList;
    caf::ChildArrayField<SmallDemoObjectA*>  m_objectListOfSameType;
    caf::PtrField<SmallDemoObjectA*>         m_ptrField;

    caf::Field<bool> m_toggleField;

    MenuItemProducer* m_menuItemProducer;

    caf::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if (changedField == &m_toggleField)
        {
            qDebug() << "Toggle Field changed";
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void onEditorWidgetsCreated() override
    {
        for (auto e : m_longText.capability<caf::FieldUiCapability>()->connectedEditors())
        {
            caf::PdmUiTextEditor* textEditor = dynamic_cast<caf::PdmUiTextEditor*>(e);
            if (!textEditor) continue;

            QWidget* containerWidget = textEditor->editorWidget();
            if (!containerWidget) continue;

            for (auto qObj : containerWidget->children())
            {
                QTextEdit* textEdit = dynamic_cast<QTextEdit*>(qObj);
                if (textEdit)
                {
                    m_menuItemProducer->attachTextEdit(textEdit);
                }
            }
        }
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineCustomContextMenu(const caf::FieldHandle* fieldNeedingMenu,
                                 caf::MenuInterface*        menu,
                                 QWidget*                   fieldEditorWidget) override
    {
        if (fieldNeedingMenu == &m_objectListOfSameType)
        {
            caf::PdmUiTableView::addActionsToMenu(menu, &m_objectListOfSameType);
        }
    }
};

CAF_PDM_SOURCE_INIT(DemoObject, "DemoObject");

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    caf::PdmUiItem::enableExtraDebugText(true);

    // Initialize command framework

    // Register default command features (add/delete item in list)

    QPixmap pix;
    pix.load(":/images/curvePlot.png");

    m_plotLabel = new QLabel(this);
    m_plotLabel->setPixmap(pix.scaled(250, 100));

    m_smallPlotLabel = new QLabel(this);
    m_smallPlotLabel->setPixmap(pix.scaled(100, 50));

    createActions();
    createDockPanels();
    buildTestModel();

    setPdmRoot(m_testRoot);

    sm_mainWindowInstance = this;
    caf::SelectionManager::instance()->setPdmRootObject(m_testRoot);

    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
    undoView->setStack(caf::CmdExecCommandManager::instance()->undoStack());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("PdmTreeView - controls property view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTreeView = new caf::PdmUiTreeView(dockWidget);
        dockWidget->setWidget(m_pdmUiTreeView);
        m_pdmUiTreeView->treeView()->setContextMenuPolicy(Qt::CustomContextMenu);

        QObject::connect(m_pdmUiTreeView->treeView(),
                         SIGNAL(customContextMenuRequested(const QPoint&)),
                         SLOT(slotCustomMenuRequestedForProjectTree(const QPoint&)));

        m_pdmUiTreeView->treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_pdmUiTreeView->enableSelectionManagerUpdating(true);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("CustomObjectEditor", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_customObjectEditor = new caf::CustomObjectEditor;
        QWidget* w           = m_customObjectEditor->getOrCreateWidget(this);
        dockWidget->setWidget(w);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafPropertyView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("PdmTreeView2  - controls table view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTreeView2 = new caf::PdmUiTreeView(dockWidget);
        m_pdmUiTreeView2->enableDefaultContextMenu(true);
        m_pdmUiTreeView2->enableSelectionManagerUpdating(true);
        dockWidget->setWidget(m_pdmUiTreeView2);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafTableView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTableView = new caf::PdmUiTableView(dockWidget);

        dockWidget->setWidget(m_pdmUiTableView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("Undo stack", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        undoView = new QUndoView(this);
        dockWidget->setWidget(undoView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::buildTestModel()
{
    m_testRoot = new DemoObjectGroup;

    ManyGroups* manyGroups = new ManyGroups;
    m_testRoot->objects.push_back(manyGroups);

    DemoObject* demoObject = new DemoObject;
    m_testRoot->objects.push_back(demoObject);

    SmallDemoObject* smallObj1 = new SmallDemoObject;
    m_testRoot->objects.push_back(smallObj1);

    SmallDemoObjectA* smallObj2 = new SmallDemoObjectA;
    m_testRoot->objects.push_back(smallObj2);

    SmallGridDemoObject* smallGridObj = new SmallGridDemoObject;
    m_testRoot->objects.push_back(smallGridObj);

    SingleEditorObject* singleEditorObj = new SingleEditorObject;
    m_testRoot->objects.push_back(singleEditorObj);

    auto tamComboBox = new TamComboBox;
    m_testRoot->objects.push_back(tamComboBox);

    DemoObject* demoObj2 = new DemoObject;

    demoObject->m_textField = "Mitt Demo Obj";
    demoObject->m_objectList.push_back(demoObj2);
    demoObject->m_objectList.push_back(new SmallDemoObjectA());
    SmallDemoObject* smallObj3 = new SmallDemoObject();
    demoObject->m_objectList.push_back(smallObj3);
    demoObject->m_objectList.push_back(new SmallDemoObject());

    demoObject->m_objectListOfSameType.push_back(new SmallDemoObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoObjectA());

    demoObj2->m_objectList.push_back(new SmallDemoObjectA());
    demoObj2->m_objectList.push_back(new SmallDemoObjectA());
    demoObj2->m_objectList.push_back(new SmallDemoObject());

    delete smallObj3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setPdmRoot(caf::ObjectHandle* pdmRoot)
{
    caf::ObjectUiCapability* uiObject = uiObj(pdmRoot);

    m_pdmUiTreeView->setPdmItem(uiObject);

    connect(m_pdmUiTreeView, SIGNAL(selectionChanged()), SLOT(slotSimpleSelectionChanged()));

    // Set up test of using a field as a root item
    // Hack, because we know that pdmRoot is a ObjectGroup ...

    std::vector<caf::FieldHandle*> fields;
    if (pdmRoot)
    {
        pdmRoot->fields(fields);
    }

    if (fields.size())
    {
        caf::FieldHandle*       field         = fields[0];
        caf::FieldUiCapability* uiFieldHandle = field->capability<caf::FieldUiCapability>();
        if (uiFieldHandle)
        {
            m_pdmUiTreeView2->setPdmItem(uiFieldHandle);
            uiFieldHandle->updateConnectedEditors();
        }
    }

    m_pdmUiTreeView2->setPdmItem(uiObject);

    connect(m_pdmUiTreeView2, SIGNAL(selectionChanged()), SLOT(slotShowTableView()));

    // Wire up ManyGroups object
    std::vector<ManyGroups*> obj;
    if (pdmRoot)
    {
        pdmRoot->descendantsIncludingThisOfType(obj);
    }

    m_customObjectEditor->removeWidget(m_plotLabel);
    m_customObjectEditor->removeWidget(m_smallPlotLabel);

    if (obj.size() == 1)
    {
        m_customObjectEditor->setObject(obj[0]);

        m_customObjectEditor->defineGridLayout(5, 4);

        m_customObjectEditor->addBlankCell(0, 0);
        m_customObjectEditor->addWidget(m_plotLabel, 0, 1, 1, 2);
        m_customObjectEditor->addWidget(m_smallPlotLabel, 1, 2, 2, 1);
    }
    else
    {
        m_customObjectEditor->setObject(nullptr);
    }

    m_customObjectEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_pdmUiTreeView->setPdmItem(nullptr);
    m_pdmUiTreeView2->setPdmItem(nullptr);
    m_pdmUiPropertyView->showProperties(nullptr);
    m_pdmUiTableView->setChildArrayField(nullptr);

    delete m_pdmUiTreeView;
    delete m_pdmUiTreeView2;
    delete m_pdmUiPropertyView;
    delete m_pdmUiTableView;
    delete m_customObjectEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    if (m_testRoot)
    {
        m_testRoot->objects.deleteAllChildObjects();
        delete m_testRoot;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow* MainWindow::instance()
{
    return sm_mainWindowInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createActions()
{
    {
        QAction* loadAction = new QAction("Load Project", this);
        QAction* saveAction = new QAction("Save Project", this);

        connect(loadAction, SIGNAL(triggered()), SLOT(slotLoadProject()));
        connect(saveAction, SIGNAL(triggered()), SLOT(slotSaveProject()));

        QMenu* menu = menuBar()->addMenu("&File");
        menu->addAction(loadAction);
        menu->addAction(saveAction);
    }

    {
        QAction* editInsert    = new QAction("&Insert", this);
        QAction* editRemove    = new QAction("&Remove", this);
        QAction* editRemoveAll = new QAction("Remove all", this);

        connect(editInsert, SIGNAL(triggered()), SLOT(slotInsert()));
        connect(editRemove, SIGNAL(triggered()), SLOT(slotRemove()));
        connect(editRemoveAll, SIGNAL(triggered()), SLOT(slotRemoveAll()));

        QMenu* menu = menuBar()->addMenu("&Edit");
        menu->addAction(editInsert);
        menu->addAction(editRemove);
        menu->addAction(editRemoveAll);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotInsert()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caf::FieldUiCapability*                      uiFh  = dynamic_cast<caf::FieldUiCapability*>(selection[i]);
        caf::ChildArrayField<caf::ObjectHandle*>* field = nullptr;

        if (uiFh) field = dynamic_cast<caf::ChildArrayField<caf::ObjectHandle*>*>(uiFh->fieldHandle());

        if (field)
        {
            field->push_back(new DemoObject);
            field->capability<caf::FieldUiCapability>()->updateConnectedEditors();

            return;
        }
#if 0
        caf::ChildArrayFieldHandle* listField = NULL;

        if (uiFh) listField = dynamic_cast<caf::ChildArrayFieldHandle*>(uiFh->fieldHandle());

        if (listField)
        {
            caf::ObjectHandle* obj = listField->createAppendObject();
            listField->capability<caf::PdmUiFieldHandle>()->updateConnectedEditors();
        }
#endif
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caf::ObjectHandle* obj = dynamic_cast<caf::ObjectHandle*>(selection[i]);
        if (obj)
        {
            caf::FieldHandle* field = obj->parentField();

            // Ordering is important

            field->removeChildObject(obj);

            // Delete object
            delete obj;

            // Update editors
            field->capability<caf::FieldUiCapability>()->updateConnectedEditors();

            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemoveAll() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSimpleSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);
    caf::ObjectHandle*          obj       = nullptr;
    caf::ChildArrayFieldHandle* listField = nullptr;

    if (selection.size())
    {
        caf::ObjectUiCapability* pdmUiObj = dynamic_cast<caf::ObjectUiCapability*>(selection[0]);
        if (pdmUiObj) obj = pdmUiObj->objectHandle();
    }

    m_pdmUiPropertyView->showProperties(obj);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotShowTableView()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView2->selectedUiItems(selection);
    caf::ObjectHandle*          obj                   = nullptr;
    caf::FieldUiCapability*     uiFieldHandle         = nullptr;
    caf::ChildArrayFieldHandle* childArrayFieldHandle = nullptr;

    if (selection.size())
    {
        caf::PdmUiItem* pdmUiItem = selection[0];

        uiFieldHandle = dynamic_cast<caf::FieldUiCapability*>(pdmUiItem);
        if (uiFieldHandle)
        {
            childArrayFieldHandle = dynamic_cast<caf::ChildArrayFieldHandle*>(uiFieldHandle->fieldHandle());
        }

        if (childArrayFieldHandle)
        {
            if (!childArrayFieldHandle->hasSameFieldCountForAllObjects())
            {
                uiFieldHandle         = nullptr;
                childArrayFieldHandle = nullptr;
            }
        }
    }

    m_pdmUiTableView->setChildArrayField(childArrayFieldHandle);

    if (uiFieldHandle)
    {
        uiFieldHandle->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotLoadProject()
{
    QString fileName =
        QFileDialog::getOpenFileName(nullptr, tr("Open Project File"), "test.proj", "Project Files (*.proj);;All files(*.*)");
    if (!fileName.isEmpty())
    {
        setPdmRoot(nullptr);
        releaseTestData();

        m_testRoot           = new DemoObjectGroup;
        m_testRoot->fileName = fileName;
        m_testRoot->readFile();

        setPdmRoot(m_testRoot);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSaveProject()
{
    QString fileName =
        QFileDialog::getSaveFileName(nullptr, tr("Save Project File"), "test.proj", "Project Files (*.proj);;All files(*.*)");
    if (!fileName.isEmpty())
    {
        m_testRoot->fileName = fileName;
        m_testRoot->writeFile();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotCustomMenuRequestedForProjectTree(const QPoint&)
{
    QObject*   senderObj = this->sender();
    QTreeView* treeView  = dynamic_cast<QTreeView*>(senderObj);
    if (treeView)
    {
        caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget(m_pdmUiTreeView);

        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "cafToggleItemsOnFeature";
        menuBuilder << "cafToggleItemsOffFeature";
        menuBuilder << "cafToggleItemsFeature";
        menuBuilder << "Separator";
        menuBuilder << "cafToggleItemsOnOthersOffFeature";

        caf::QMenuWrapper menuWrapper;
        menuBuilder.appendToMenu(&menuWrapper);

        menuWrapper.menu()->exec(QCursor::pos());
        caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget(nullptr);
    }
}
