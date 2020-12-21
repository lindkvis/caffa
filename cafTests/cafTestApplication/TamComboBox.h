
#pragma once

#include "cafAppEnum.h"
#include "cafField.h"
#include "cafObject.h"

#include <QStringList>

//==================================================================================================
///
///
//==================================================================================================
class TamComboBox : public caf::Object
{
    CAF_HEADER_INIT;

public:
    TamComboBox();

    virtual std::deque<caf::OptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                                  bool*                   useOptionsOnly) override;

    void onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                    const caf::FieldCapability* changedCapability,
                                    const caf::Variant&         oldValue,
                                    const caf::Variant&         newValue) override;

private:
    caf::Field<std::string> m_name;

protected:
    virtual void defineUiOrdering(caf::UiOrdering& uiOrdering) override;

    virtual void defineEditorAttribute(const caf::FieldHandle* field, caf::UiEditorAttribute* attribute) override;

private:
    std::deque<std::string> m_historyItems;
};
