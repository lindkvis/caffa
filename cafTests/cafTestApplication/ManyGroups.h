#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafProxyValueField.h"

class ManyGroups : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    ManyGroups();

    caf::Field<double>           m_doubleField;
    caf::Field<int>              m_intField;
    caf::Field<QString>          m_textField;
    caf::ProxyValueField<double> m_proxyDoubleField;

    caf::Field<std::vector<QString>> m_multiSelectList;
    caf::Field<QString>              m_stringWithMultipleOptions;

    caf::Field<bool>  m_toggleField;
    caf::FieldHandle* objectToggleField() override;

    void fieldChangedByUi(const caf::FieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void setDoubleMember(const double& d)
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    void defineEditorAttribute(const caf::FieldHandle* field,
                               QString                    uiConfigName,
                               caf::UiEditorAttribute* attribute) override;
};
