
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
#include "cafQActionWrapper.h"
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

class DemoObjectGroup : public caffa::Document
{
    CAFFA_HEADER_INIT;

public:
    DemoObjectGroup()
    {
        assignUiInfo("Project", "", "Test Project", "");
        initField(objects, "Objects").withUi();
        initField(m_textField, "Description").withUi("Project");
        objects.capability<caffa::FieldUiCapability>()->setUiHidden(true);
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caffa::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

public:
    caffa::ChildArrayField<ObjectHandle*> objects;
    caffa::Field<std::string>             m_textField;
};

CAFFA_SOURCE_INIT(DemoObjectGroup, "DemoObjectGroup", "Document", "Object");

class TinyDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    TinyDemoObject();

private:
    caffa::Field<bool>   m_toggleField;
    caffa::Field<double> m_doubleField;
};

CAFFA_SOURCE_INIT(TinyDemoObject, "TinyDemoObject", "Object");

TinyDemoObject::TinyDemoObject()
{
    assignUiInfo("Tiny Demo Object", "", "This object is a demo of the CAF framework", "");
    initField(m_toggleField, "Toggle").withDefault(false).withUi("Toggle Item", "", "Tooltip", " Whatsthis?");
    initField(m_doubleField, "Number")
        .withDefault(0.0)
        .withUi("Number", "", "Enter a floating point number here", "Double precision floating point number");
}

class SmallDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

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
        m_doubleField.capability<caffa::FieldUiCapability>()->setCustomContextMenuEnabled(true);

        initField(m_intField, "IntNumber", 0)
            .withUi("Small Number",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        initField(m_textField, "TextField")
            .withUi("Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

        auto proxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
        proxyAccessor->registerSetMethod(this, &SmallDemoObject::setDoubleMember);
        proxyAccessor->registerGetMethod(this, &SmallDemoObject::doubleMember);
        initField(m_proxyDoubleField, "ProxyDouble").withUi("Proxy Double").withAccessor(std::move(proxyAccessor));

        m_proxyDoubleField = 0.0;
        if (!(m_proxyDoubleField == 3.0))
        {
            qDebug() << "Double is not 3 ";
        }

        initField(m_multiSelectList, "SelectedItems").withUi("Multi Select Field", "", "", "");
        m_multiSelectList.capability<caffa::FieldIoCapability>()->setIOReadable(false);
        m_multiSelectList.capability<caffa::FieldIoCapability>()->setIOWritable(false);
        m_multiSelectList.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(
            caffa::UiTreeSelectionEditor::uiEditorTypeName());

        m_multiSelectList = {"First", "Second", "Third"};
    }

    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_textField;

    caffa::Field<double> m_proxyDoubleField;

    caffa::Field<std::vector<std::string>> m_multiSelectList;

    caffa::Field<bool>  m_toggleField;
    caffa::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                    const caffa::FieldCapability* changedCapability,
                                    const caffa::Variant&         oldValue,
                                    const caffa::Variant&         newValue) override
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
    std::deque<caffa::OptionItemInfo> calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                            bool*                     useOptionsOnly) override
    {
        std::deque<caffa::OptionItemInfo> options;

        if (fieldNeedingOptions == &m_multiSelectList)
        {
            std::string text;

            text = "First";
            options.push_back(caffa::OptionItemInfo(text, text));

            text = "Second";
            options.push_back(caffa::OptionItemInfo::createHeader(
                text, false, std::make_shared<caffa::IconProvider>(":/images/win/textbold.png")));

            {
                text                           = "Second_a";
                caffa::OptionItemInfo itemInfo = caffa::OptionItemInfo(text, text, true);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            {
                text = "Second_b";
                caffa::OptionItemInfo itemInfo =
                    caffa::OptionItemInfo(text, text, false, std::make_shared<caffa::IconProvider>(":/images/win/filenew.png"));
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            int additionalSubItems = 2;
            for (auto i = 0; i < additionalSubItems; i++)
            {
                text                           = "Second_b_" + std::to_string(i);
                caffa::OptionItemInfo itemInfo = caffa::OptionItemInfo(text, text);
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
                text                           = "Second_b_" + std::to_string(i);
                caffa::OptionItemInfo itemInfo = caffa::OptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            text = "Third";
            options.push_back(caffa::OptionItemInfo(text, text));

            text = "Fourth";
            options.push_back(caffa::OptionItemInfo(text, text));
        }

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    /*    void defineCustomContextMenu(const caffa::FieldHandle* fieldNeedingMenu,
                                     caffa::MenuInterface*     menu,
                                     QWidget*                fieldEditorWidget) override
        {
            menu->addAction(caffa::CmdFeatureManager::instance()->action("test", "nothing"));
            menu->addAction(caffa::CmdFeatureManager::instance()->action("other test <<>>", "still nothing"));
        }
        */
private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caffa::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_doubleField);
        uiOrdering.add(&m_intField);
        std::string dynamicGroupName = std::string("Dynamic Group Text (") + std::to_string(m_intField()) + ")";

        caffa::UiGroup* group = uiOrdering.addNewGroupWithKeyword(dynamicGroupName, "MyTest");
        group->add(&m_textField);
        group->add(&m_proxyDoubleField);
    }
};

CAFFA_SOURCE_INIT(SmallDemoObject, "SmallDemoObject", "Object");

class SmallGridDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

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
        m_intFieldLabelTop.capability<caffa::FieldUiCapability>()->setUiLabelPosition(caffa::UiItemInfo::TOP);
        initField(m_stringFieldLabelHidden, "FieldLabelHidden", "Hidden Label Field")
            .withUi("Field Label Hidden",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHidden.capability<caffa::FieldUiCapability>()->setUiLabelPosition(caffa::UiItemInfo::HIDDEN);

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
        m_intFieldLabelTopAuto.capability<caffa::FieldUiCapability>()->setUiLabelPosition(caffa::UiItemInfo::TOP);
        initField(m_stringFieldLabelHiddenAuto, "FieldLabelHiddenAuto", "Hidden Label Field")
            .withUi("Field Label Hidden",
                    "",
                    "Enter some small number here",
                    "This is a place you can enter a small integer value if you want");
        m_stringFieldLabelHiddenAuto.capability<caffa::FieldUiCapability>()->setUiLabelPosition(caffa::UiItemInfo::HIDDEN);

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
    caffa::Field<int> m_intFieldStandard;
    caffa::Field<int> m_intFieldUseFullSpace;
    caffa::Field<int> m_intFieldUseFullSpaceLabel;
    caffa::Field<int> m_intFieldUseFullSpaceField;
    caffa::Field<int> m_intFieldWideLabel;
    caffa::Field<int> m_intFieldWideField;
    caffa::Field<int> m_intFieldWideBoth;
    caffa::Field<int> m_intFieldLeft;
    caffa::Field<int> m_intFieldRight;

    // First group
    caffa::Field<int>         m_intFieldWideBoth2;
    caffa::Field<int>         m_intFieldLeft2;
    caffa::Field<int>         m_intFieldCenter;
    caffa::Field<int>         m_intFieldRight2;
    caffa::Field<int>         m_intFieldLabelTop;
    caffa::Field<std::string> m_stringFieldLabelHidden;

    // Auto group
    caffa::Field<int>         m_intFieldWideBothAuto;
    caffa::Field<int>         m_intFieldLeftAuto;
    caffa::Field<int>         m_intFieldCenterAuto;
    caffa::Field<int>         m_intFieldRightAuto;
    caffa::Field<int>         m_intFieldLabelTopAuto;
    caffa::Field<std::string> m_stringFieldLabelHiddenAuto;

    // Combination with groups
    caffa::Field<int> m_intFieldLeftOfGroup;
    caffa::Field<int> m_intFieldRightOfGroup;
    caffa::Field<int> m_intFieldInsideGroup1;
    caffa::Field<int> m_intFieldInsideGroup2;

    // Side-by-side groups
    caffa::Field<int> m_intFieldInsideGroup3;
    caffa::Field<int> m_intFieldInsideGroup4;
    caffa::Field<int> m_intFieldInsideGroup5;
    caffa::Field<int> m_intFieldInsideGroup6;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caffa::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
        uiOrdering.add(&m_intFieldUseFullSpace,
                       caffa::UiOrdering::LayoutOptions(true,
                                                        caffa::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN,
                                                        caffa::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceLabel,
                       caffa::UiOrdering::LayoutOptions(true, 3, caffa::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN));
        uiOrdering.add(&m_intFieldUseFullSpaceField,
                       caffa::UiOrdering::LayoutOptions(true, caffa::UiOrdering::LayoutOptions::MAX_COLUMN_SPAN, 1));
        uiOrdering.add(&m_intFieldWideLabel, caffa::UiOrdering::LayoutOptions(true, 4, 3));
        uiOrdering.add(&m_intFieldWideField, caffa::UiOrdering::LayoutOptions(true, 4, 1));
        uiOrdering.add(&m_intFieldLeft, caffa::UiOrdering::LayoutOptions(true));
        uiOrdering.add(&m_intFieldRight, caffa::UiOrdering::LayoutOptions(false));
        uiOrdering.add(&m_intFieldWideBoth, caffa::UiOrdering::LayoutOptions(true, 4, 2));

        std::string dynamicGroupName = std::string("Dynamic Group Text (") + std::to_string(m_intFieldStandard) + ")";

        caffa::UiGroup* group = uiOrdering.addNewGroup("Wide Group", {true, 4});
        group->add(&m_intFieldWideBoth2, caffa::UiOrdering::LayoutOptions(true, 6, 3));
        group->add(&m_intFieldLeft2, caffa::UiOrdering::LayoutOptions(true));
        group->add(&m_intFieldCenter, caffa::UiOrdering::LayoutOptions(false));
        group->add(&m_intFieldRight2, caffa::UiOrdering::LayoutOptions(false));
        group->add(&m_intFieldLabelTop, caffa::UiOrdering::LayoutOptions(true, 6));
        group->add(&m_stringFieldLabelHidden, caffa::UiOrdering::LayoutOptions(true, 6));

        caffa::UiGroup* autoGroup = uiOrdering.addNewGroup("Automatic Full Width Group", caffa::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldWideBothAuto, caffa::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldLeftAuto, caffa::UiOrdering::LayoutOptions(true));
        autoGroup->add(&m_intFieldCenterAuto, false);
        autoGroup->add(&m_intFieldRightAuto, caffa::UiOrdering::LayoutOptions(false));
        autoGroup->add(&m_intFieldLabelTopAuto, true);
        autoGroup->add(&m_stringFieldLabelHiddenAuto, true);

        uiOrdering.add(&m_intFieldLeftOfGroup);
        caffa::UiGroup* group2 = uiOrdering.addNewGroup("Right Group", caffa::UiOrdering::LayoutOptions(false, 2, 0));
        group2->setEnableFrame(false);
        group2->add(&m_intFieldInsideGroup1);

        caffa::UiGroup* group3 = uiOrdering.addNewGroup("Narrow L", caffa::UiOrdering::LayoutOptions(true, 1));
        group3->add(&m_intFieldInsideGroup2);
        uiOrdering.add(&m_intFieldRightOfGroup, caffa::UiOrdering::LayoutOptions(false, 3, 2));

        caffa::UiGroup* groupL = uiOrdering.addNewGroup("Left Group", caffa::UiOrdering::LayoutOptions(true, 1));
        groupL->add(&m_intFieldInsideGroup3);
        groupL->add(&m_intFieldInsideGroup5);
        caffa::UiGroup* groupR = uiOrdering.addNewGroup("Right Wide Group", caffa::UiOrdering::LayoutOptions(false, 3));
        groupR->setEnableFrame(false);
        groupR->add(&m_intFieldInsideGroup4);
        groupR->add(&m_intFieldInsideGroup6);
    }
};

CAFFA_SOURCE_INIT(SmallGridDemoObject, "SmallGridDemoObject", "Object");

class SingleEditorObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

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
    caffa::Field<int> m_intFieldStandard;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caffa::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
    }
};

CAFFA_SOURCE_INIT(SingleEditorObject, "SingleEditorObject", "Object");

class SmallDemoObjectA : public caffa::Object
{
    CAFFA_HEADER_INIT;

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
        initField(m_testEnumField, "TestEnumValue", caffa::AppEnum<TestEnumType>(T1)).withUi("EnumField", "", "", "");

        initField(m_proxyEnumField, "ProxyEnumValue").withUi("ProxyEnum", "", "", "");
        auto enumProxyAccessor = std::make_unique<caffa::FieldProxyAccessor<caffa::AppEnum<TestEnumType>>>();

        enumProxyAccessor->registerSetMethod(this, &SmallDemoObjectA::setEnumMember);
        enumProxyAccessor->registerGetMethod(this, &SmallDemoObjectA::enumMember);
        m_proxyEnumField.setFieldDataAccessor(std::move(enumProxyAccessor));
        m_proxyEnumMember = T2;

        m_testEnumField.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(caffa::UiListEditor::uiEditorTypeName());

        initField(m_multipleAppEnum, "MultipleAppEnumValue").withUi("MultipleAppEnumValue", "", "", "");
        m_multipleAppEnum.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(
            caffa::UiTreeSelectionEditor::uiEditorTypeName());
        initField(m_highlightedEnum, "HighlightedEnum").withUi("HighlightedEnum", "", "", "");
        m_highlightedEnum.capability<caffa::FieldUiCapability>()->setUiHidden(true);
    }

    caffa::Field<double>                       m_doubleField;
    caffa::Field<int>                          m_intField;
    caffa::Field<std::string>                  m_textField;
    caffa::Field<caffa::AppEnum<TestEnumType>> m_testEnumField;

    caffa::DataValueField<caffa::AppEnum<TestEnumType>> m_proxyEnumField;
    void                                                setEnumMember(const caffa::AppEnum<TestEnumType>& val)
    {
        m_proxyEnumMember = val.value();
    }
    caffa::AppEnum<TestEnumType> enumMember() const
    {
        return m_proxyEnumMember;
    }
    TestEnumType m_proxyEnumMember;

    // vector of app enum
    caffa::Field<std::vector<caffa::AppEnum<TestEnumType>>> m_multipleAppEnum;
    caffa::Field<caffa::AppEnum<TestEnumType>>              m_highlightedEnum;

    caffa::Field<bool> m_toggleField;
    caffa::Field<bool> m_pushButtonField;

    caffa::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                    const caffa::FieldCapability* changedCapability,
                                    const caffa::Variant&         oldValue,
                                    const caffa::Variant&         newValue) override
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

    std::deque<caffa::OptionItemInfo> calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                            bool*                     useOptionsOnly) override
    {
        std::deque<caffa::OptionItemInfo> options;

        if (&m_multipleAppEnum == fieldNeedingOptions)
        {
            for (size_t i = 0; i < caffa::AppEnum<TestEnumType>::size(); ++i)
            {
                options.push_back(caffa::OptionItemInfo(caffa::AppEnum<TestEnumType>::uiTextFromIndex(i),
                                                        caffa::AppEnum<TestEnumType>::fromIndex(i)));
            }
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caffa::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineEditorAttribute(const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute) override
    {
        if (field == &m_multipleAppEnum)
        {
            caffa::UiTreeSelectionEditorAttribute* attr = dynamic_cast<caffa::UiTreeSelectionEditorAttribute*>(attribute);
            if (attr)
            {
                attr->currentIndexFieldHandle = &m_highlightedEnum;
            }
        }
        else if (field == &m_proxyEnumField)
        {
            caffa::UiComboBoxEditorAttribute* attr = dynamic_cast<caffa::UiComboBoxEditorAttribute*>(attribute);
            if (attr)
            {
                attr->showPreviousAndNextButtons = true;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineObjectEditorAttribute(caffa::UiEditorAttribute* attribute) override
    {
        caffa::UiTableViewPushButtonEditorAttribute* attr = dynamic_cast<caffa::UiTableViewPushButtonEditorAttribute*>(attribute);
        if (attr)
        {
            attr->registerPushButtonTextForFieldKeyword(m_pushButtonField.keyword(), "Edit");
        }
    }
};

CAFFA_SOURCE_INIT(SmallDemoObjectA, "SmallDemoObjectA", "Object");

namespace caffa
{
template<>
void AppEnum<SmallDemoObjectA::TestEnumType>::setUp()
{
    addItem(SmallDemoObjectA::T1, "T1", "An A letter");
    addItem(SmallDemoObjectA::T2, "T2", "A B letter");
    addItem(SmallDemoObjectA::T3, "T3", "A B C letter");
    setDefault(SmallDemoObjectA::T1);
}

} // namespace caffa

class DemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

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
        m_objectListOfSameType.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(
            caffa::UiTableViewEditor::uiEditorTypeName());
        m_objectListOfSameType.capability<caffa::FieldUiCapability>()->setCustomContextMenuEnabled(true);

        m_longText.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(caffa::UiTextEditor::uiEditorTypeName());
        m_longText.capability<caffa::FieldUiCapability>()->setUiLabelPosition(caffa::UiItemInfo::HIDDEN);

        m_menuItemProducer = new MenuItemProducer;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caffa::UiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_objectListOfSameType);
        uiOrdering.add(&m_boolField);
        caffa::UiGroup* group1 = uiOrdering.addNewGroup("Name1");
        group1->add(&m_doubleField);
        caffa::UiGroup* group2 = uiOrdering.addNewGroup("Name2");
        group2->add(&m_intField);
        caffa::UiGroup* group3 = group2->addNewGroup("Name3");
        // group3->add(&m_textField);

        uiOrdering.skipRemainingFields();
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    std::deque<caffa::OptionItemInfo> calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                            bool*                     useOptionsOnly) override
    {
        std::deque<caffa::OptionItemInfo> options;
        if (&m_multiSelectList == fieldNeedingOptions)
        {
            options.push_back(caffa::OptionItemInfo("Choice 1", "Choice1"));
            options.push_back(caffa::OptionItemInfo("Choice 2", "Choice2"));
            options.push_back(caffa::OptionItemInfo("Choice 3", "Choice3"));
            options.push_back(caffa::OptionItemInfo("Choice 4", "Choice4"));
            options.push_back(caffa::OptionItemInfo("Choice 5", "Choice5"));
            options.push_back(caffa::OptionItemInfo("Choice 6", "Choice6"));
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    caffa::FieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

    // Fields
    caffa::Field<bool>        m_boolField;
    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_textField;

    caffa::Field<std::string>              m_longText;
    caffa::Field<std::vector<std::string>> m_multiSelectList;

    caffa::ChildArrayField<caffa::ObjectHandle*> m_objectList;
    caffa::ChildArrayField<SmallDemoObjectA*>    m_objectListOfSameType;

    caffa::Field<bool> m_toggleField;

    MenuItemProducer* m_menuItemProducer;

    caffa::FieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    void onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                    const caffa::FieldCapability* changedCapability,
                                    const caffa::Variant&         oldValue,
                                    const caffa::Variant&         newValue) override
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
        for (auto e : m_longText.capability<caffa::FieldUiCapability>()->connectedEditors())
        {
            caffa::UiTextEditor* textEditor = dynamic_cast<caffa::UiTextEditor*>(e);
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
    /* void defineCustomContextMenu(const caffa::FieldHandle* fieldNeedingMenu,
                                 caffa::MenuInterface*     menu,
                                 QWidget*                fieldEditorWidget) override
    {
        if (fieldNeedingMenu == &m_objectListOfSameType)
        {
            caffa::UiTableView::addActionsToMenu(menu, &m_objectListOfSameType);
        }
    } */
};

CAFFA_SOURCE_INIT(DemoObject, "DemoObject", "Object");

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    caffa::UiItem::enableExtraDebugText(true);

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
    caffa::SelectionManager::instance()->setRootObject(m_testRoot.get());

    // caffa::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
    // undoView->setStack(caffa::CmdExecCommandManager::instance()->undoStack());
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

        m_uiTreeView = new caffa::UiTreeView(dockWidget);
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

        m_customObjectEditor = new caffa::CustomObjectEditor;
        QWidget* w           = m_customObjectEditor->getOrCreateWidget(this);
        dockWidget->setWidget(w);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafPropertyView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiPropertyView = new caffa::UiPropertyView(dockWidget);
        dockWidget->setWidget(m_uiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("TreeView2  - controls table view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiTreeView2 = new caffa::UiTreeView(dockWidget);
        m_uiTreeView2->enableDefaultContextMenu(true);
        m_uiTreeView2->enableSelectionManagerUpdating(true);
        dockWidget->setWidget(m_uiTreeView2);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafTableView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_uiTableView = new caffa::UiTableView(dockWidget);

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
void MainWindow::setRoot(caffa::ObjectHandle* root)
{
    caffa::ObjectUiCapability* uiObject = uiObj(root);

    m_uiTreeView->setItem(uiObject);

    connect(m_uiTreeView, SIGNAL(selectionChanged()), SLOT(slotSimpleSelectionChanged()));

    // Set up test of using a field as a root item
    // Hack, because we know that root is a ObjectGroup ...

    if (root)
    {
        auto fields = root->fields();

        if (!fields.empty())
        {
            caffa::FieldHandle*       field         = fields.front();
            caffa::FieldUiCapability* uiFieldHandle = field->capability<caffa::FieldUiCapability>();
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
    std::vector<caffa::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caffa::FieldUiCapability*                     uiFh  = dynamic_cast<caffa::FieldUiCapability*>(selection[i]);
        caffa::ChildArrayField<caffa::ObjectHandle*>* field = nullptr;

        if (uiFh) field = dynamic_cast<caffa::ChildArrayField<caffa::ObjectHandle*>*>(uiFh->fieldHandle());

        if (field)
        {
            field->push_back(std::make_unique<DemoObject>());
            field->capability<caffa::FieldUiCapability>()->updateConnectedEditors();

            return;
        }
#if 0
        caffa::ChildArrayFieldHandle* listField = nullptr;

        if (uiFh) listField = dynamic_cast<caffa::ChildArrayFieldHandle*>(uiFh->fieldHandle());

        if (listField)
        {
            caffa::ObjectHandle* obj = listField->createAppendObject();
            listField->capability<caffa::UiFieldHandle>()->updateConnectedEditors();
        }
#endif
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    std::vector<caffa::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caffa::ObjectHandle* obj = dynamic_cast<caffa::ObjectHandle*>(selection[i]);
        if (obj)
        {
            caffa::FieldHandle* field = obj->parentField();

            // Ordering is important

            auto childObject = field->removeChildObject(obj);

            // Update editors
            field->capability<caffa::FieldUiCapability>()->updateConnectedEditors();

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
    std::vector<caffa::UiItem*> selection;
    m_uiTreeView->selectedUiItems(selection);
    caffa::ObjectHandle*          obj       = nullptr;
    caffa::ChildArrayFieldHandle* listField = nullptr;

    if (selection.size())
    {
        caffa::ObjectUiCapability* uiObj = dynamic_cast<caffa::ObjectUiCapability*>(selection[0]);
        if (uiObj) obj = uiObj->objectHandle();
    }

    m_uiPropertyView->showProperties(obj);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotShowTableView()
{
    std::vector<caffa::UiItem*> selection;
    m_uiTreeView2->selectedUiItems(selection);
    caffa::ObjectHandle*          obj                   = nullptr;
    caffa::FieldUiCapability*     uiFieldHandle         = nullptr;
    caffa::ChildArrayFieldHandle* childArrayFieldHandle = nullptr;

    if (selection.size())
    {
        caffa::UiItem* uiItem = selection[0];

        uiFieldHandle = dynamic_cast<caffa::FieldUiCapability*>(uiItem);
        if (uiFieldHandle)
        {
            childArrayFieldHandle = dynamic_cast<caffa::ChildArrayFieldHandle*>(uiFieldHandle->fieldHandle());
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
        caffa::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget(m_uiTreeView);

        caffa::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "cafToggleItemsOnFeature";
        menuBuilder << "cafToggleItemsOffFeature";
        menuBuilder << "cafToggleItemsFeature";
        menuBuilder << "Separator";
        menuBuilder << "cafToggleItemsOnOthersOffFeature";

        caffa::QMenuWrapper menuWrapper;
        menuBuilder.appendToMenu(&menuWrapper);

        menuWrapper.menu()->exec(QCursor::pos());
        caffa::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget(nullptr);
    }
} */
