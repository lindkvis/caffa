
#include "ManyGroups.h"
#include "cafUiListEditor.h"
#include "cafUiTreeSelectionEditor.h"
#include "cafVariant.h"

CAF_SOURCE_INIT(ManyGroups, "LargeObject");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ManyGroups::ManyGroups()
{
    CAF_InitObject("Many Groups",
                   ":/images/win/filenew.png",
                   "This object is a demo of the CAF framework",
                   "This object is a demo of the CAF framework");

    CAF_InitField(
        &m_toggleField, "Toggle", false, "Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
    CAF_InitField(&m_doubleField,
                  "BigNumber",
                  0.0,
                  "Big Number",
                  "",
                  "Enter a big number here",
                  "This is a place you can enter a big real value if you want");
    CAF_InitField(&m_intField,
                  "IntNumber",
                  0,
                  "Small Number",
                  "",
                  "Enter some small number here",
                  "This is a place you can enter a small integer value if you want");
    CAF_InitField(&m_textField,
                  "TextField",
                  std::string(""),
                  "Text",
                  "",
                  "Text tooltip",
                  "This is a place you can enter a small integer value if you want");

    CAF_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");
    auto proxyAccessor = std::make_unique<caf::FieldProxyAccessor<double>>();
    proxyAccessor->registerSetMethod(this, &ManyGroups::setDoubleMember);
    proxyAccessor->registerGetMethod(this, &ManyGroups::doubleMember);
    m_proxyDoubleField.setFieldDataAccessor(std::move(proxyAccessor));

    m_doubleField      = 0.0;
    m_proxyDoubleField = 0.0;
    if (!(m_proxyDoubleField == 3.0))
    {
        std::cout << "Double is not 3 " << std::endl;
    }

    CAF_InitFieldNoDefault(&m_multiSelectList, "SelectedItems", "Multi Select Field", "", "", "");

    m_multiSelectList.capability<caf::FieldIoCapability>()->setIOReadable(false);
    m_multiSelectList.capability<caf::FieldIoCapability>()->setIOWritable(false);
    m_multiSelectList.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
        caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

    m_multiSelectList = {"First", "Second", "Third"};

    CAF_InitField(
        &m_stringWithMultipleOptions, "m_stringWithMultipleOptions", std::string(""), "Text with many items", "", "", "");
    m_stringWithMultipleOptions.capability<caf::FieldUiCapability>()->setUiEditorTypeName(
        caf::PdmUiListEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FieldHandle* ManyGroups::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ManyGroups::onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                            const caf::FieldCapability* changedFieldCapability,
                                            const caf::Variant&         oldValue,
                                            const caf::Variant&         newValue)
{
    if (changedField == &m_toggleField)
    {
        std::cout << "Toggle Field changed" << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<caf::OptionItemInfo> ManyGroups::calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                                  bool*                   useOptionsOnly)
{
    std::deque<caf::OptionItemInfo> options;

    // Test code used to switch between two lists with different content, but same item count
    if (fieldNeedingOptions == &m_stringWithMultipleOptions)
    {
        for (int i = 0; i < 100; i++)
        {
            std::string text = std::string("item ") + std::to_string(i);
            options.push_back(caf::OptionItemInfo(text, text));
        }

        return options;
    }

    // Test code used to switch between two lists with different content, but same item count
    if (fieldNeedingOptions == &m_multiSelectList)
    {
        if (m_intField < 10)
        {
            std::string text = "Apple 1";
            options.push_back(caf::OptionItemInfo(text, text));

            text = "Apple 2";
            options.push_back(caf::OptionItemInfo(text, text));
        }
        else
        {
            std::string text = "Car 1";
            options.push_back(caf::OptionItemInfo(text, text));

            text = "Car 2";
            options.push_back(caf::OptionItemInfo(text, text));
        }

        return options;
    }

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
void ManyGroups::defineUiOrdering(caf::UiOrdering& uiOrdering)
{
    uiOrdering.add(&m_toggleField);
    uiOrdering.add(&m_multiSelectList);

    /*
        {
            caf::UiGroup* group = uiOrdering.addNewGroup("First");

            caf::UiGroup* subGroup = group->addNewGroup("First_Content");

            subGroup->add(&m_doubleField);
            subGroup->add(&m_intField);

            caf::UiGroup* subGroup2 = group->addNewGroup("First_Content_2");

        }

        {
            caf::UiGroup* group = uiOrdering.addNewGroup("Second");
            caf::UiGroup* subGroup = group->addNewGroup("Second_Content");
        }

        {
            caf::UiGroup* group = uiOrdering.addNewGroup("Third");
            caf::UiGroup* subGroup = group->addNewGroup("Third_Content");
        }

        {
            caf::UiGroup* group = uiOrdering.addNewGroup("Fourth");
            caf::UiGroup* subGroup = group->addNewGroup("Fourth_Content");

            subGroup->add(&m_textField);
        }

        {
            caf::UiGroup* group = uiOrdering.addNewGroup("Fifth");
            caf::UiGroup* subGroup = group->addNewGroup("Fifth_Content");

            subGroup->add(&m_proxyDoubleField);
        }
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ManyGroups::defineEditorAttribute(const caf::FieldHandle* field, caf::UiEditorAttribute* attribute)
{
    if (field == &m_multiSelectList)
    {
        caf::PdmUiTreeSelectionEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>(attribute);
        if (myAttr)
        {
            // myAttr->showTextFilter = false;
            // myAttr->showToggleAllCheckbox = false;
        }
    }
}
