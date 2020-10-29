
#include "TamComboBox.h"

#include "cafUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT(TamComboBox, "TamComboBox");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TamComboBox::TamComboBox()
{
    CAF_PDM_InitObject("Cell Filter", "", "", "");

    CAF_PDM_InitField(&m_name, "UserDescription", QString("Filter Name"), "Name", "", "", "");
    m_name.capability<caf::FieldUiCapability>()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> TamComboBox::calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    for (const auto& s : m_historyItems)
    {
        options.push_back(caf::PdmOptionItemInfo(s, s));
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_name)
    {
        QString text = m_name();

        if (m_historyItems.indexOf(text) == -1)
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
void TamComboBox::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TamComboBox::defineEditorAttribute(const caf::FieldHandle* field,
                                        QString                    uiConfigName,
                                        caf::UiEditorAttribute* attribute)
{
    auto attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>(attribute);
    if (attr)
    {
        attr->enableEditableContent = true;
    }
}
