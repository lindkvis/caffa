
#include "TamComboBox.h"

#include "cafUiComboBoxEditor.h"

CAF_SOURCE_INIT(TamComboBox, "TamComboBox");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TamComboBox::TamComboBox()
{
    assignUiInfo("Cell Filter", "", "", "");

    initField(m_name, "UserDescription", "Filter Name").withUi("Name", "", "", "");
    m_name.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::UiComboBoxEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<caf::OptionItemInfo> TamComboBox::calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                                   bool*                   useOptionsOnly)
{
    std::deque<caf::OptionItemInfo> options;

    for (const auto& s : m_historyItems)
    {
        options.push_back(caf::OptionItemInfo(s, s));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                             const caf::FieldCapability* changedCapability,
                                             const caf::Variant&         oldValue,
                                             const caf::Variant&         newValue)
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
void TamComboBox::defineUiOrdering(caf::UiOrdering& uiOrdering) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::defineEditorAttribute(const caf::FieldHandle* field, caf::UiEditorAttribute* attribute)
{
    auto attr = dynamic_cast<caf::UiComboBoxEditorAttribute*>(attribute);
    if (attr)
    {
        attr->enableEditableContent = true;
    }
}
