
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

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    virtual void
        fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::Field<QString> m_name;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual void defineEditorAttribute(const caf::FieldHandle* field,
                                       QString                    uiConfigName,
                                       caf::UiEditorAttribute* attribute) override;

private:
    QStringList m_historyItems;
};
