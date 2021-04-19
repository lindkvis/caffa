
#pragma once

#include "cafAppEnum.h"
#include "cafField.h"
#include "cafObject.h"

#include <QStringList>

//==================================================================================================
///
///
//==================================================================================================
class TamComboBox : public caffa::Object
{
    CAF_HEADER_INIT;

public:
    TamComboBox();

    virtual std::deque<caffa::OptionItemInfo> calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                                  bool*                   useOptionsOnly) override;

    void onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                    const caffa::FieldCapability* changedCapability,
                                    const caffa::Variant&         oldValue,
                                    const caffa::Variant&         newValue) override;

private:
    caffa::Field<std::string> m_name;

protected:
    virtual void defineUiOrdering(caffa::UiOrdering& uiOrdering) override;

    virtual void defineEditorAttribute(const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute) override;

private:
    std::deque<std::string> m_historyItems;
};
