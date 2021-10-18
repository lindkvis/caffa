
#include "TamComboBox.h"

#include "cafUiComboBoxEditor.h"

CAFFA_SOURCE_INIT(TamComboBox, "TamComboBox", "Object");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TamComboBox::TamComboBox()
{
    assignUiInfo("Cell Filter", "", "");

    initField(m_name, "UserDescription", "Filter Name").withUi("Name", "", "");
    m_name.capability<caffa::FieldUiCapability>()->setUiEditorTypeName(caffa::UiComboBoxEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<caffa::OptionItemInfo> TamComboBox::calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                                     bool*                     useOptionsOnly)
{
    std::deque<caffa::OptionItemInfo> options;

    for (const auto& s : m_historyItems)
    {
        options.push_back(caffa::OptionItemInfo(s, s));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                             const caffa::FieldCapability* changedCapability,
                                             const caffa::Variant&         oldValue,
                                             const caffa::Variant&         newValue)
{
    if (changedField == &m_name)
    {
        auto text = m_name();

        if (std::find(m_historyItems.begin(), m_historyItems.end(), text) == m_historyItems.end())
        {
            m_historyItems.push_front(m_name);
            while (m_historyItems.size() > 5)
            {
                m_historyItems.pop_back();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::defineUiOrdering(caffa::UiOrdering& uiOrdering) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::defineEditorAttribute(const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute)
{
    auto attr = dynamic_cast<caffa::UiComboBoxEditorAttribute*>(attribute);
    if (attr)
    {
        attr->enableEditableContent = true;
    }
}
