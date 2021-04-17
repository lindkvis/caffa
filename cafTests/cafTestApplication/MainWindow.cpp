
#include "cafField.h"

#include "MainWindow.h"

#include "CustomObjectEditor.h"
#include "ManyGroups.h"
#include "MenuItemProducer.h"
#include "TamComboBox.h"
#include "WidgetLayoutTest.h"

#include "cafAppEnum.h"

#include "cafDocument.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldUiCapability.h"
#include "cafObject.h"
#include "cafObjectGroup.h"
#include "cafPtrField.h"
#include "cafQActionWrapper.h"
#include "cafReferenceHelper.h"
#include "cafSelectionManager.h"
#include "cafUiComboBoxEditor.h"
#include "cafUiItem.h"
#include "cafUiListEditor.h"
#include "cafUiOrdering.h"
#include "cafUiPropertyView.h"
#include "cafUiPushButtonEditor.h"
#include "cafUiTableView.h"
#include "cafUiTableViewEditor.h"
#include "cafUiTextEditor.h"
#include "cafUiTreeSelectionEditor.h"
#include "cafUiTreeView.h"

#include <QAction>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QUndoView>

#include <filesystem>

class DemoObjectGroup : public caf::Document
{
    CAF_HEADER_INIT;

public:
    DemoObjectGroup()
    {
        assignUiInfo("Project", "", "Test Project", "");
        initField(objects, "Objects").withUi();
        initField(m_textField, "Description").withUi("Project");
        objects.capability<caf::FieldUiCapability>()->setUiHidden(true);
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caf::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

public:
    caf::ChildArrayField<ObjectHandle*> objects;
    caf::Field<std::string>             m_textField;
};

CAF_SOURCE_INIT(DemoObjectGroup, "DemoObjectGroup");

class TinyDemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    TinyDemoObject();

private:
    caf::Field<bool>   m_toggleField;
    caf::Field<double> m_doubleField;
};

CAF_SOURCE_INIT(TinyDemoObject, "TinyDemoObject");

TinyDemoObject::TinyDemoObject()
{
    assignUiInfo("Tiny Demo Object", "", "This object is a demo of the CAF framework", "");
    initField(m_toggleField, "Toggle").withDefault(false).withUi("Toggle Item", "", "Tooltip", " Whatsthis?");
    initField(m_doubleField, "Number")
        .withDefault(0.0)
        .withUi("Number", "", "Enter a floating point number here", "Double precision floating point number");
}

class SmallDemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SmallDemoObject()
    {
        assignUiInfo("Small Demo Object",
                     ":/images/win/filenew.png",
                     "This object is a demo of the CAF framework",
                     "This object is a demo of the CAF framework");

        initField(m_toggleField, "Toggle", false)
            .withUi("Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        initField(m_doubleField, "BigNumber", 0.0)
            .withUi("Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        m_doubleField.capability<caf::FieldUiCapability>()->setCustomContextMenuEnabled(true);

        initField(m_intField, "IntNumber", 0)
            .withUi("Small Number",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_textField, "TextField")
            .withUi("Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

        auto proxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
        proxyAccessor->registerSetMethod(this, &SmallDemoObject::setDoubleMember);
        proxyAccessor->registerGetMethod(this, &SmallDemoObject::doubleMember);
        initField(m_proxyDoubleField, "ProxyDouble").withUi("Proxy Double").withAccessor(std::move(proxyAccessor));

        m_proxyDoubleField = 0.0;
        if (!(m_proxyDoubleField == 3.0))
        {
            qDebug() << "Double is not 3 ";
        }

        initField(m_multiSelectList, "SelectedItems").withUi("Multi Select Field", "", "", "");
        m_multiSelectList.capability<caf::FieldIoCapability>()->setIOReadable(false);
        m_multiSelectList.capability<caf::FieldIoCapability>()->setIOWritable(false);
        m_multiSelectList.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::UiTreeSelectionEditor::uiEditorTypeName());

        m_multiSelectList = {"First", "Second", "Third"};
    }

    caf::Field<double>      m_doubleField;
    caf::Field<int>         m_intField;
    caf::Field<std::string> m_textField;

    caf::Field<double> m_proxyDoubleField;

    caf::Field<std::vector<std::string>> m_multiSelectList;

    caf::Field<bool>  m_toggleField;
    caf::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                    const caf::FieldCapability* changedCapability,
                                    const caf::Variant&         oldValue,
                                    const caf::Variant&         newValue) override
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
    std::deque<caf::OptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                          bool*                   useOptionsOnly) override
    {
        std::deque<caf::OptionItemInfo> options;

        if (fieldNeedingOptions == &m_multiSelectList)
        {
            std::string text;

            text = "First";
            options.push_back(caf::OptionItemInfo(text, text));

            text = "Second";
            options.push_back(
                caf::OptionItemInfo::createHeader(text, false, std::make_shared<caf::IconProvider>(":/images/win/textbold.png")));

            {
                text                         = "Second_a";
                caf::OptionItemInfo itemInfo = caf::OptionItemInfo(text, text, true);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            {
                text = "Second_b";
                caf::OptionItemInfo itemInfo =
                    caf::OptionItemInfo(text, text, false, std::make_shared<caf::IconProvider>(":/images/win/filenew.png"));
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            int additionalSubItems = 2;
            for (auto i = 0; i < additionalSubItems; i++)
            {
                text                         = "Second_b_" + std::to_string(i);
                caf::OptionItemInfo itemInfo = caf::OptionItemInfo(text, text);
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
                text                         = "Second_b_" + std::to_string(i);
                caf::OptionItemInfo itemInfo = caf::OptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            text = "Third";
            options.push_back(caf::OptionItemInfo(text, text));

            text = "Fourth";
            options.push_back(caf::OptionItemInfo(text, text));
        }

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    /*    void defineCustomContextMenu(const caf::FieldHandle* fieldNeedingMenu,
                                     caf::MenuInterface*     menu,
                                     QWidget*                fieldEditorWidget) override
        {
            menu->addAction(caf::CmdFeatureManager::instance()->action("test", "nothing"));
            menu->addAction(caf::CmdFeatureManager::instance()->action("other test <<>>", "still nothing"));
        }
        */
private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caf::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_doubleField);
        uiOrdering.add(&m_intField);
        std::string dynamicGroupName = std::string("Dynamic Group Text (") + std::to_string(m_intField()) + ")";

        caf::UiGroup* group = uiOrdering.addNewGroupWithKeyword(dynamicGroupName, "MyTest");
        group->add(&m_textField);
        group->add(&m_proxyDoubleField);
    }
};

CAF_SOURCE_INIT(SmallDemoObject, "SmallDemoObject");

class SmallGridDemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SmallGridDemoObject()
    {
        assignUiInfo("Small Grid Demo Object",
                     "",
                     "This object is a demo of the CAF framework",
                     "This object is a demo of the CAF framework");

        initField(m_intFieldStandard, "Standard", 0)
            .withUi("Standard",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldUseFullSpace, "FullSpace", 0)
            .withUi("Use Full Space For Both",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldUseFullSpaceLabel, "FullSpaceLabel", 0)
            .withUi("Total 3, Label MAX",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldUseFullSpaceField, "FullSpaceField", 0)
            .withUi("Total MAX, Label 1",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldWideLabel, "WideLabel", 0)
            .withUi("Wide Label",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldWideField, "WideField", 0)
            .withUi("Wide Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldLeft, "LeftField", 0)
            .withUi("Left Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldRight, "RightField", 0)
            .withUi("Right Field With More Text",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldWideBoth, "WideBoth", 0)
            .withUi("Wide Both",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");

        initField(m_intFieldWideBoth2, "WideBoth2", 0)
            .withUi("Wide Both",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldLeft2, "LeftFieldInGrp", 0)
            .withUi("Left Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldCenter, "CenterFieldInGrp", 0)
            .withUi("Center Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldRight2, "RightFieldInGrp", 0)
            .withUi("Right Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldLabelTop, "FieldLabelTop", 0)
            .withUi("Field Label Top",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_intFieldLabelTop.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::UiItemInfo::TOP);
        initField(m_stringFieldLabelHidden, "FieldLabelHidden", "Hidden Label Field")
            .withUi("Field Label Hidden",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHidden.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::UiItemInfo::HIDDEN);

        initField(m_intFieldWideBothAuto, "WideBothAuto", 0)
            .withUi(
                "Wide ", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
        initField(m_intFieldLeftAuto, "LeftFieldInGrpAuto", 0)
            .withUi("Left Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldCenterAuto, "CenterFieldInGrpAuto", 0)
            .withUi("Center Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldRightAuto, "RightFieldInGrpAuto", 0)
            .withUi("Right Field",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldLabelTopAuto, "FieldLabelTopAuto", 0)
            .withUi("Field Label Top",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_intFieldLabelTopAuto.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::UiItemInfo::TOP);
        initField(m_stringFieldLabelHiddenAuto, "FieldLabelHiddenAuto", "Hidden Label Field")
            .withUi("Field Label Hidden",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHiddenAuto.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::UiItemInfo::HIDDEN);

        initField(m_intFieldLeftOfGroup, "FieldLeftOfGrp", 0)
            .withUi("Left of group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldRightOfGroup, "FieldRightOfGrp", 0)
            .withUi("Right of group wide label",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");

        initField(m_intFieldInsideGroup1, "FieldInGrp1", 0)
            .withUi("Inside Group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldInsideGroup2, "FieldInGrp2", 0)
            .withUi("Inside Group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldInsideGroup3, "FieldInGrp3", 0)
            .withUi("Inside Group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldInsideGroup4, "FieldInGrp4", 0)
            .withUi("Inside Group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldInsideGroup5, "FieldInGrp5", 0)
            .withUi("Inside Group",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_intFieldInsideGroup6, "FieldInGrp6", 0)
            .withUi("Inside Group",
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
    caf::Field<int>         m_intFieldWideBoth2;
    caf::Field<int>         m_intFieldLeft2;
    caf::Field<int>         m_intFieldCenter;
    caf::Field<int>         m_intFieldRight2;
    caf::Field<int>         m_intFieldLabelTop;
    caf::Field<std::string> m_stringFieldLabelHidden;

    // Auto group
    caf::Field<int>         m_intFieldWideBothAuto;
    caf::Field<int>         m_intFieldLeftAuto;
    caf::Field<int>         m_intFieldCenterAuto;
    caf::Field<int>         m_intFieldRightAuto;
    caf::Field<int>         m_intFieldLabelTopAuto;
    caf::Field<std::string> m_stringFieldLabelHiddenAuto;

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
    void defineUiOrdering(caf::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
        uiOrdering.add(&m_intFieldUseFullSpace,
                       caf::UiOrdering::LayoutOptions(true,
                                                      caf::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN,
                                                      caf::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceLabel,
                       caf::UiOrdering::LayoutOptions(true, 3, caf::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceField,
                       caf::UiOrdering::LayoutOptions(true, caf::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1));
        uiOrdering.add(&m_intFieldWideLabel, caf::UiOrdering::LayoutOptions(true, 4, 3));
        uiOrdering.add(&m_intFieldWideField, caf::UiOrdering::LayoutOptions(true, 4, 1));
        uiOrdering.add(&m_intFieldLeft, caf::UiOrdering::LayoutOptions(true));
        uiOrdering.add(&m_intFieldRight, caf::UiOrdering::LayoutOptions(false));
        uiOrdering.add(&m_intFieldWideBoth, caf::UiOrdering::LayoutOptions(true, 4, 2));

        std::string dynamicGroupName = std::string("Dynamic Group Text (") + std::to_string(m_intFieldStandard) + ")";

        caf::UiGroup* group = uiOrdering.addNewGroup("Wide Group", {true, 4});
        group->add(&m_intFieldWideBoth2, caf::UiOrdering::LayoutOptions(true, 6, 3));
        group->add(&m_intFieldLeft2, caf::UiOrdering::LayoutOptions(true));
        group->add(&m_intFieldCenter, caf::UiOrdering::LayoutOptions(false));
        group->add(&m_intFieldRight2, caf::UiOrdering::LayoutOptions(false));
        group->add(&m_intFieldLabelTop, caf::UiOrdering::LayoutOptions(true, 6));
        group->add(&m_stringFieldLabelHidden, caf::UiOrdering::LayoutOptions(true, 6));

        caf::UiGroup* autoGroup = uiOrdering.addNewGroup("Automatic Full Width Group", caf::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldWideBothAuto, caf::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldLeftAuto, caf::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldCenterAuto, false);
        autoGroup->add(&m_intFieldRightAuto, caf::UiOrdering::LayoutOptions(false));
        autoGroup->add(&m_intFieldLabelTopAuto, true);
        autoGroup->add(&m_stringFieldLabelHiddenAuto, true);

        uiOrdering.add(&m_intFieldLeftOfGroup);
        caf::UiGroup* group2 = uiOrdering.addNewGroup("Right Group", caf::UiOrdering::LayoutOptions(false, 2, 0));
        group2->setEnableFrame(false);
        group2->add(&m_intFieldInsideGroup1);

        caf::UiGroup* group3 = uiOrdering.addNewGroup("Narrow L", caf::UiOrdering::LayoutOptions(true, 1));
        group3->add(&m_intFieldInsideGroup2);
        uiOrdering.add(&m_intFieldRightOfGroup, caf::UiOrdering::LayoutOptions(false, 3, 2));

        caf::UiGroup* groupL = uiOrdering.addNewGroup("Left Group", caf::UiOrdering::LayoutOptions(true, 1));
        groupL->add(&m_intFieldInsideGroup3);
        groupL->add(&m_intFieldInsideGroup5);
        caf::UiGroup* groupR = uiOrdering.addNewGroup("Right Wide Group", caf::UiOrdering::LayoutOptions(false, 3));
        groupR->setEnableFrame(false);
        groupR->add(&m_intFieldInsideGroup4);
        groupR->add(&m_intFieldInsideGroup6);
    }
};

CAF_SOURCE_INIT(SmallGridDemoObject, "SmallGridDemoObject");

class SingleEditorObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    SingleEditorObject()
    {
        assignUiInfo("Single Editor Object",
                     "",
                     "This object is a demo of the CAF framework",
                     "This object is a demo of the CAF framework");

        initField(m_intFieldStandard, "Standard", 0)
            .withUi("Fairly Wide Label",
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
    void defineUiOrdering(caf::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
    }
};

CAF_SOURCE_INIT(SingleEditorObject, "SingleEditorObject");

class SmallDemoObjectA : public caf::Object
{
    CAF_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    SmallDemoObjectA()
    {
        assignUiInfo("Small Demo Object A",
                     "",
                     "This object is a demo of the CAF framework",
                     "This object is a demo of the CAF framework");

        initField(m_toggleField, "Toggle", false).withUi("Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        initField(m_pushButtonField, "Push", false).withUi("Button Field", "", "", " ");
        initField(m_doubleField, "BigNumber", 0.0)
            .withUi("Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        initField(m_intField, "IntNumber", 0)
            .withUi("Small Number",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_textField, "TextField", "Small Demo Object A").withUi("Name Text Field", "", "", "");
        initField(m_testEnumField, "TestEnumValue", caf::AppEnum<TestEnumType>(T1)).withUi("EnumField", "", "", "");
        initField(m_ptrField, "m_ptrField").withUi("PtrField", "", "", "");

        initField(m_proxyEnumField, "ProxyEnumValue").withUi("ProxyEnum", "", "", "");
        auto enumProxyAccessor = std::make_unique<caf::FieldProxyAccessor<caf::AppEnum<TestEnumType>>>();

        enumProxyAccessor->registerSetMethod(this, &SmallDemoObjectA::setEnumMember);
        enumProxyAccessor->registerGetMethod(this, &SmallDemoObjectA::enumMember);
        m_proxyEnumField.setFieldDataAccessor(std::move(enumProxyAccessor));
        m_proxyEnumMember = T2;

        m_testEnumField.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::UiListEditor::uiEditorTypeName());

        initField(m_multipleAppEnum, "MultipleAppEnumValue").withUi("MultipleAppEnumValue", "", "", "");
        m_multipleAppEnum.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::UiTreeSelectionEditor::uiEditorTypeName());
        initField(m_highlightedEnum, "HighlightedEnum").withUi("HighlightedEnum", "", "", "");
        m_highlightedEnum.capability<caf::FieldUiCapability>()->setUiHidden(true);
    }

    caf::Field<double>                     m_doubleField;
    caf::Field<int>                        m_intField;
    caf::Field<std::string>                m_textField;
    caf::Field<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::PtrField<SmallDemoObjectA*>       m_ptrField;

    caf::DataValueField<caf::AppEnum<TestEnumType>> m_proxyEnumField;
    void                                            setEnumMember(const caf::AppEnum<TestEnumType>& val)
    {
        m_proxyEnumMember = val.value();
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

    void onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                    const caf::FieldCapability* changedCapability,
                                    const caf::Variant&         oldValue,
                                    const caf::Variant&         newValue) override
    {
        if (changedField == &m_toggleField)
        {
            qDebug() << "Toggle Field changed";
        }
        else if (changedField == &m_highlightedEnum)
        {
            qDebug() << "Highlight value " << QString::fromStdString(m_highlightedEnum().text());
        }
        else if (changedField == &m_pushButtonField)
        {
            qDebug() << "Push Button pressed ";
        }
    }

    std::deque<caf::OptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                          bool*                   useOptionsOnly) override
    {
        std::deque<caf::OptionItemInfo> options;

        if (&m_ptrField == fieldNeedingOptions)
        {
            caf::FieldHandle*               field;
            std::vector<caf::ObjectHandle*> objects;
            field = this->parentField();

            field->childObjects(&objects);

            for (size_t i = 0; i < objects.size(); ++i)
            {
                std::string userDesc;

                caf::ObjectUiCapability* uiObject = caf::uiObj(objects[i]);
                if (uiObject)
                {
                    if (objects[i]->userDescriptionField())
                    {
                        caf::FieldUiCapability* uiFieldHandle =
                            objects[i]->userDescriptionField()->capability<caf::FieldUiCapability>();
                        if (uiFieldHandle)
                        {
                            userDesc = uiFieldHandle->uiValue().value<std::string>();
                        }
                    }

                    options.push_back(caf::OptionItemInfo(userDesc, caf::Variant(caf::Pointer<caf::ObjectHandle>(objects[i]))));
                }
            }
        }
        else if (&m_multipleAppEnum == fieldNeedingOptions)
        {
            for (size_t i = 0; i < caf::AppEnum<TestEnumType>::size(); ++i)
            {
                options.push_back(caf::OptionItemInfo(caf::AppEnum<TestEnumType>::uiTextFromIndex(i),
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
    void defineEditorAttribute(const caf::FieldHandle* field, caf::UiEditorAttribute* attribute) override
    {
        if (field == &m_multipleAppEnum)
        {
            caf::UiTreeSelectionEditorAttribute* attr = dynamic_cast<caf::UiTreeSelectionEditorAttribute*>(attribute);
            if (attr)
            {
                attr->currentIndexFieldHandle = &m_highlightedEnum;
            }
        }
        else if (field == &m_proxyEnumField)
        {
            caf::UiComboBoxEditorAttribute* attr = dynamic_cast<caf::UiComboBoxEditorAttribute*>(attribute);
            if (attr)
            {
                attr->showPreviousAndNextButtons = true;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineObjectEditorAttribute(caf::UiEditorAttribute* attribute) override
    {
        caf::UiTableViewPushButtonEditorAttribute* attr = dynamic_cast<caf::UiTableViewPushButtonEditorAttribute*>(attribute);
        if (attr)
        {
            attr->registerPushButtonTextForFieldKeyword(m_pushButtonField.keyword(), "Edit");
        }
    }
};

CAF_SOURCE_INIT(SmallDemoObjectA, "SmallDemoObjectA");

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

class DemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    DemoObject()
    {
        assignUiInfo(
            "Demo Object", "", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

        initField(m_toggleField, "Toggle", false).withUi("Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        initField(m_doubleField, "BigNumber", 0.0)
            .withUi("Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        initField(m_intField, "IntNumber", 0)
            .withUi("Small Number",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_boolField, "BooleanValue", false)
            .withUi("Boolean:",
                    "",
                    "Boolean:Enter some small number here",
                    "Boolean:This is a place you can enter a small integer value if you want");
        initField(m_textField, "TextField", "Demo Object Description Field").withUi("Description Field", "", "", "");
        initField(m_longText, "LongText", "Test text").withUi("Long Text", "", "", "");

        initField(m_multiSelectList, "MultiSelect").withUi("Selection List", "", "List", "This is a multi selection list");
        initField(m_objectList, "ObjectList").withUi("Objects list Field", "", "List", "This is a list of Objects");
        initField(m_objectListOfSameType, "m_objectListOfSameType")
            .withUi("Same type Objects list Field", "", "Same type List", "Same type list of Objects");
        m_objectListOfSameType.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
            caf::UiTableViewEditor::uiEditorTypeName());
        m_objectListOfSameType.capability<caf::FieldUiCapability>()->setCustomContextMenuEnabled(true);
        ;
        initField(m_ptrField, "m_ptrField").withUi("PtrField", "", "Same type List", "Same type list of Objects");

        m_longText.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::UiTextEditor::uiEditorTypeName());
        m_longText.capability<caf::FieldUiCapability>()->setUiLabelPosition(caf::UiItemInfo::HIDDEN);

        m_menuItemProducer = new MenuItemProducer;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caf::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_objectListOfSameType);
        uiOrdering.add(&m_ptrField);
        uiOrdering.add(&m_boolField);
        caf::UiGroup* group1 = uiOrdering.addNewGroup("Name1");
        group1->add(&m_doubleField);
        caf::UiGroup* group2 = uiOrdering.addNewGroup("Name2");
        group2->add(&m_intField);
        caf::UiGroup* group3 = group2->addNewGroup("Name3");
        // group3->add(&m_textField);

        uiOrdering.skipRemainingFields();
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::deque<caf::OptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                          bool*                   useOptionsOnly) override
    {
        std::deque<caf::OptionItemInfo> options;
        if (&m_multiSelectList == fieldNeedingOptions)
        {
            options.push_back(caf::OptionItemInfo("Choice 1", "Choice1"));
            options.push_back(caf::OptionItemInfo("Choice 2", "Choice2"));
            options.push_back(caf::OptionItemInfo("Choice 3", "Choice3"));
            options.push_back(caf::OptionItemInfo("Choice 4", "Choice4"));
            options.push_back(caf::OptionItemInfo("Choice 5", "Choice5"));
            options.push_back(caf::OptionItemInfo("Choice 6", "Choice6"));
        }

        if (&m_ptrField == fieldNeedingOptions)
        {
            for (size_t i = 0; i < m_objectListOfSameType.size(); ++i)
            {
                caf::ObjectUiCapability* uiObject = caf::uiObj(m_objectListOfSameType[i]);
                if (uiObject)
                {
                    std::string userDesc;

                    caf::FieldUiCapability* uiFieldHandle = this->userDescriptionField()->capability<caf::FieldUiCapability>();
                    if (uiFieldHandle)
                    {
                        userDesc = uiFieldHandle->uiValue().value<std::string>();
                    }
                    options.push_back(
                        caf::OptionItemInfo(userDesc, caf::Variant(caf::Pointer<caf::ObjectHandle>(m_objectListOfSameType[i]))));
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
    caf::Field<bool>        m_boolField;
    caf::Field<double>      m_doubleField;
    caf::Field<int>         m_intField;
    caf::Field<std::string> m_textField;

    caf::Field<std::string>              m_longText;
    caf::Field<std::vector<std::string>> m_multiSelectList;

    caf::ChildArrayField<caf::ObjectHandle*> m_objectList;
    caf::ChildArrayField<SmallDemoObjectA*>  m_objectListOfSameType;
    caf::PtrField<SmallDemoObjectA*>         m_ptrField;

    caf::Field<bool> m_toggleField;

    MenuItemProducer* m_menuItemProducer;

    caf::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                    const caf::FieldCapability* changedCapability,
                                    const caf::Variant&         oldValue,
                                    const caf::Variant&         newValue) override
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
            caf::UiTextEditor* textEditor = dynamic_cast<caf::UiTextEditor*>(e);
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
    /* void defineCustomContextMenu(const caf::FieldHandle* fieldNeedingMenu,
                                 caf::MenuInterface*     menu,
                                 QWidget*                fieldEditorWidget) override
    {
        if (fieldNeedingMenu == &m_objectListOfSameType)
        {
            caf::UiTableView::addActionsToMenu(menu, &m_objectListOfSameType);
        }
    } */
};

CAF_SOURCE_INIT(DemoObject, "DemoObject");

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    caf::UiItem::enableExtraDebugText(true);

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

    setRoot(m_testRoot.get());

    sm_mainWindowInstance = this;
    caf::SelectionManager::instance()->setRootObject(m_testRoot.get());

    // caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
    // undoView->setStack(caf::CmdExecCommandManager::instance()->undoStack());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("TreeView - controls property view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiTreeView = new caf::UiTreeView(dockWidget);
        dockWidget->setWidget(m_uiTreeView);
        m_uiTreeView->treeView()->setContextMenuPolicy(Qt::CustomContextMenu);
        /* QObject::connect(m_uiTreeView->treeView(),
                         SIGNAL(customContextMenuRequested(const QPoint&)),
                         SLOT(slotCustomMenuRequestedForProjectTree(const QPoint&))); */

        m_uiTreeView->treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_uiTreeView->enableSelectionManagerUpdating(true);

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

        m_uiPropertyView = new caf::UiPropertyView(dockWidget);
        dockWidget->setWidget(m_uiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("TreeView2  - controls table view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiTreeView2 = new caf::UiTreeView(dockWidget);
        m_uiTreeView2->enableDefaultContextMenu(true);
        m_uiTreeView2->enableSelectionManagerUpdating(true);
        dockWidget->setWidget(m_uiTreeView2);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafTableView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiTableView = new caf::UiTableView(dockWidget);

        dockWidget->setWidget(m_uiTableView);

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
    m_testRoot = std::make_unique<DemoObjectGroup>();

    auto manyGroups = std::make_unique<ManyGroups>();
    m_testRoot->objects.push_back(std::move(manyGroups));

    auto demoObject    = std::make_unique<DemoObject>();
    auto demoObjectPtr = demoObject.get();
    m_testRoot->objects.push_back(std::move(demoObject));

    auto smallObj1 = std::make_unique<SmallDemoObject>();
    m_testRoot->objects.push_back(std::move(smallObj1));

    auto smallObj2 = std::make_unique<SmallDemoObjectA>();
    m_testRoot->objects.push_back(std::move(smallObj2));

    auto smallGridObj = std::make_unique<SmallGridDemoObject>();
    m_testRoot->objects.push_back(std::move(smallGridObj));

    auto singleEditorObj = std::make_unique<SingleEditorObject>();
    m_testRoot->objects.push_back(std::move(singleEditorObj));

    auto tamComboBox = std::make_unique<TamComboBox>();
    m_testRoot->objects.push_back(std::move(tamComboBox));

    auto demoObj2              = std::make_unique<DemoObject>();
    auto demoObj2Ptr           = demoObj2.get();
    demoObjectPtr->m_textField = "Mitt Demo Obj";
    demoObjectPtr->m_objectList.push_back(std::move(demoObj2));
    demoObjectPtr->m_objectList.push_back(std::make_unique<SmallDemoObjectA>());
    demoObjectPtr->m_objectList.push_back(std::make_unique<SmallDemoObject>());
    demoObjectPtr->m_objectList.push_back(std::make_unique<SmallDemoObject>());

    demoObjectPtr->m_objectListOfSameType.push_back(std::make_unique<SmallDemoObjectA>());
    demoObjectPtr->m_objectListOfSameType.push_back(std::make_unique<SmallDemoObjectA>());
    demoObjectPtr->m_objectListOfSameType.push_back(std::make_unique<SmallDemoObjectA>());
    demoObjectPtr->m_objectListOfSameType.push_back(std::make_unique<SmallDemoObjectA>());

    demoObj2Ptr->m_objectList.push_back(std::make_unique<SmallDemoObjectA>());
    demoObj2Ptr->m_objectList.push_back(std::make_unique<SmallDemoObject>());
    demoObj2Ptr->m_objectList.push_back(std::make_unique<SmallDemoObject>());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setRoot(caf::ObjectHandle* root)
{
    caf::ObjectUiCapability* uiObject = uiObj(root);

    m_uiTreeView->setItem(uiObject);

    connect(m_uiTreeView, SIGNAL(selectionChanged()), SLOT(slotSimpleSelectionChanged()));

    // Set up test of using a field as a root item
    // Hack, because we know that root is a ObjectGroup ...

    if (root)
    {
        auto fields = root->fields();

        if (!fields.empty())
        {
            caf::FieldHandle*       field         = fields.front();
            caf::FieldUiCapability* uiFieldHandle = field->capability<caf::FieldUiCapability>();
            if (uiFieldHandle)
            {
                m_uiTreeView2->setItem(uiFieldHandle);
                uiFieldHandle->updateConnectedEditors();
            }
        }
    }

    m_uiTreeView2->setItem(uiObject);

    connect(m_uiTreeView2, SIGNAL(selectionChanged()), SLOT(slotShowTableView()));

    // Wire up ManyGroups object
    std::list<ManyGroups*> manyGroups;
    if (root)
    {
        if (auto rootGroup = dynamic_cast<ManyGroups*>(root); rootGroup)
        {
            manyGroups.push_back(rootGroup);
        }
        std::list<ManyGroups*> descendants = root->descendantsOfType<ManyGroups>();
        manyGroups.insert(manyGroups.end(), descendants.begin(), descendants.end());
    }

    m_customObjectEditor->removeWidget(m_plotLabel);
    m_customObjectEditor->removeWidget(m_smallPlotLabel);

    if (manyGroups.size() == 1)
    {
        m_customObjectEditor->setObject(manyGroups.front());

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
    m_uiTreeView->setItem(nullptr);
    m_uiTreeView2->setItem(nullptr);
    m_uiPropertyView->showProperties(nullptr);
    m_uiTableView->setChildArrayField(nullptr);

    delete m_uiTreeView;
    delete m_uiTreeView2;
    delete m_uiPropertyView;
    delete m_uiTableView;
    delete m_customObjectEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    if (m_testRoot)
    {
        m_testRoot.reset();
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
    std::vector<caf::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caf::FieldUiCapability*                   uiFh  = dynamic_cast<caf::FieldUiCapability*>(selection[i]);
        caf::ChildArrayField<caf::ObjectHandle*>* field = nullptr;

        if (uiFh) field = dynamic_cast<caf::ChildArrayField<caf::ObjectHandle*>*>(uiFh->fieldHandle());

        if (field)
        {
            field->push_back(std::make_unique<DemoObject>());
            field->capability<caf::FieldUiCapability>()->updateConnectedEditors();

            return;
        }
#if 0
        caf::ChildArrayFieldHandle* listField = nullptr;

        if (uiFh) listField = dynamic_cast<caf::ChildArrayFieldHandle*>(uiFh->fieldHandle());

        if (listField)
        {
            caf::ObjectHandle* obj = listField->createAppendObject();
            listField->capability<caf::UiFieldHandle>()->updateConnectedEditors();
        }
#endif
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    std::vector<caf::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caf::ObjectHandle* obj = dynamic_cast<caf::ObjectHandle*>(selection[i]);
        if (obj)
        {
            caf::FieldHandle* field = obj->parentField();

            // Ordering is important

            auto childObject = field->removeChildObject(obj);

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
    std::vector<caf::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);
    caf::ObjectHandle*          obj       = nullptr;
    caf::ChildArrayFieldHandle* listField = nullptr;

    if (selection.size())
    {
        caf::ObjectUiCapability* uiObj = dynamic_cast<caf::ObjectUiCapability*>(selection[0]);
        if (uiObj) obj = uiObj->objectHandle();
    }

    m_uiPropertyView->showProperties(obj);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotShowTableView()
{
    std::vector<caf::UiItem*> selection;
    m_uiTreeView2->selectedUiItems(selection);
    caf::ObjectHandle*          obj                   = nullptr;
    caf::FieldUiCapability*     uiFieldHandle         = nullptr;
    caf::ChildArrayFieldHandle* childArrayFieldHandle = nullptr;

    if (selection.size())
    {
        caf::UiItem* uiItem = selection[0];

        uiFieldHandle = dynamic_cast<caf::FieldUiCapability*>(uiItem);
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

    m_uiTableView->setChildArrayField(childArrayFieldHandle);

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
    std::string fileName =
        QFileDialog::getOpenFileName(nullptr, tr("Open Project File"), "test.proj", "Project Files (*.proj);;All files(*.*)")
            .toStdString();
    if (!fileName.empty())
    {
        setRoot(nullptr);
        releaseTestData();

        m_testRoot           = std::make_unique<DemoObjectGroup>();
        m_testRoot->fileName = fileName;
        m_testRoot->read();

        setRoot(m_testRoot.get());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSaveProject()
{
    std::string fileName =
        QFileDialog::getSaveFileName(nullptr, tr("Save Project File"), "test.proj", "Project Files (*.proj);;All files(*.*)")
            .toStdString();
    if (!fileName.empty())
    {
        m_testRoot->fileName = fileName;
        m_testRoot->write();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/* void MainWindow::slotCustomMenuRequestedForProjectTree(const QPoint&)
{
    QObject*   senderObj = this->sender();
    QTreeView* treeView  = dynamic_cast<QTreeView*>(senderObj);
    if (treeView)
    {
        caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget(m_uiTreeView);

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
} */
