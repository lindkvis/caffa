
#include "ManyGroups.h"
#include "cafUiListEditor.h"
#include "cafUiTreeSelectionEditor.h"
#include "cafVariant.h"

CAF_SOURCE_INIT(ManyGroups, "LargeObject", "Object");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ManyGroups::ManyGroups()
{
    assignUiInfo("Many Groups",
                 ":/images/win/filenew.png",
                 "This object is a demo of the CAF framework",
                 "This object is a demo of the CAF framework");

    initField(m_toggleField, "Toggle", false)
        .withUi("Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
    initField(m_doubleField, "BigNumber", 0.0)
        .withUi("Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
    initField(m_intField, "IntNumber", 0)
        .withUi("Small Number",
                "",
                "Enter some small number here",
                "This is a place you can enter a small integer value if you want");
    initField(m_textField, "TextField")
        .withUi("Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

    initField(m_proxyDoubleField, "ProxyDouble").withUi("Proxy Double", "", "", "");
    auto proxyAccessor = std::make_unique<caffa::FieldProxyAccessor<double>>();
    proxyAccessor->registerSetMethod(this, &ManyGroups::setDoubleMember);
    proxyAccessor->registerGetMethod(this, &ManyGroups::doubleMember);
    m_proxyDoubleField.setFieldDataAccessor(std::move(proxyAccessor));

    m_doubleField      = 0.0;
    m_proxyDoubleField = 0.0;
    if (!(m_proxyDoubleField == 3.0))
    {
        std::cout << "Double is not 3 " << std::endl;
    }

    initField(m_multiSelectList, "SelectedItems").withUi("Multi Select Field", "", "", "");

    m_multiSelectList.capability<caffa::FieldIoCapability>()->setIOReadable(false);
    m_multiSelectList.capability<caffa::FieldIoCapability>()->setIOWritable(false);
    m_multiSelectList.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(caffa::UiTreeSelectionEditor::uiEditorTypeName());

    m_multiSelectList = {"First", "Second", "Third"};

    initField(m_stringWithMultipleOptions, "m_stringWithMultipleOptions").withUi("Text with many items", "", "", "");
    m_stringWithMultipleOptions.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(caffa::UiListEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::FieldHandle* ManyGroups::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ManyGroups::onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                            const caffa::FieldCapability* changedFieldCapability,
                                            const caffa::Variant&         oldValue,
                                            const caffa::Variant&         newValue)
{
    if (changedField == &m_toggleField)
    {
        std::cout << "Toggle Field changed" << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<caffa::OptionItemInfo> ManyGroups::calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                                  bool*                   useOptionsOnly)
{
    std::deque<caffa::OptionItemInfo> options;

    // Test code used to switch between two lists with different content, but same item count
    if (fieldNeedingOptions == &m_stringWithMultipleOptions)
    {
        for (int i = 0; i < 100; i++)
        {
            std::string text = std::string("item ") + std::to_string(i);
            options.push_back(caffa::OptionItemInfo(text, text));
        }

        return options;
    }

    // Test code used to switch between two lists with different content, but same item count
    if (fieldNeedingOptions == &m_multiSelectList)
    {
        if (m_intField < 10)
        {
            std::string text = "Apple 1";
            options.push_back(caffa::OptionItemInfo(text, text));

            text = "Apple 2";
            options.push_back(caffa::OptionItemInfo(text, text));
        }
        else
        {
            std::string text = "Car 1";
            options.push_back(caffa::OptionItemInfo(text, text));

            text = "Car 2";
            options.push_back(caffa::OptionItemInfo(text, text));
        }

        return options;
    }

    if (fieldNeedingOptions == &m_multiSelectList)
    {
        std::string text;

        text = "First";
        options.push_back(caffa::OptionItemInfo(text, text));

        text = "Second";
        options.push_back(
            caffa::OptionItemInfo::createHeader(text, false, std::make_shared<caffa::IconProvider>(":/images/win/textbold.png")));

        {
            text                         = "Second_a";
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
            text                         = "Second_b_" + std::to_string(i);
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
            text                         = "Second_b_" + std::to_string(i);
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
void ManyGroups::defineUiOrdering(caffa::UiOrdering& uiOrdering)
{
    uiOrdering.add(&m_toggleField);
    uiOrdering.add(&m_multiSelectList);

    /*
        {
            caffa::UiGroup* group = uiOrdering.addNewGroup("First");

            caffa::UiGroup* subGroup = group->addNewGroup("First_Content");

            subGroup->add(&m_doubleField);
            subGroup->add(&m_intField);

            caffa::UiGroup* subGroup2 = group->addNewGroup("First_Content_2");

        }

        {
            caffa::UiGroup* group = uiOrdering.addNewGroup("Second");
            caffa::UiGroup* subGroup = group->addNewGroup("Second_Content");
        }

        {
            caffa::UiGroup* group = uiOrdering.addNewGroup("Third");
            caffa::UiGroup* subGroup = group->addNewGroup("Third_Content");
        }

        {
            caffa::UiGroup* group = uiOrdering.addNewGroup("Fourth");
            caffa::UiGroup* subGroup = group->addNewGroup("Fourth_Content");

            subGroup->add(&m_textField);
        }

        {
            caffa::UiGroup* group = uiOrdering.addNewGroup("Fifth");
            caffa::UiGroup* subGroup = group->addNewGroup("Fifth_Content");

            subGroup->add(&m_proxyDoubleField);
        }
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ManyGroups::defineEditorAttribute(const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute)
{
    if (field == &m_multiSelectList)
    {
        caffa::UiTreeSelectionEditorAttribute* myAttr = dynamic_cast<caffa::UiTreeSelectionEditorAttribute*>(attribute);
        if (myAttr)
        {
            // myAttr->showTextFilter = false;
            // myAttr->showToggleAllCheckbox = false;
        }
    }
}
