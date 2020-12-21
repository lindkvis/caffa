#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafProxyValueField.h"

namespace caf
{
class Variant;
}

class ManyGroups : public caf::Object
{
    CAF_HEADER_INIT;

public:
    ManyGroups();

    caf::Field<double>           m_doubleField;
    caf::Field<int>              m_intField;
    caf::Field<std::string>      m_textField;
    caf::ProxyValueField<double> m_proxyDoubleField;

    caf::Field<std::vector<std::string>> m_multiSelectList;
    caf::Field<std::string>              m_stringWithMultipleOptions;

    caf::Field<bool>  m_toggleField;
    caf::FieldHandle* objectToggleField() override;

    void onFieldChangedByCapability(const caf::FieldHandle*     changedField,
                                    const caf::FieldCapability* changedFieldCapability,
                                    const caf::Variant&         oldValue,
                                    const caf::Variant&         newValue) override;

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
    std::deque<caf::OptionItemInfo> calculateValueOptions(const caf::FieldHandle* fieldNeedingOptions,
                                                          bool*                   useOptionsOnly) override;

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caf::UiOrdering& uiOrdering) override;

    void defineEditorAttribute(const caf::FieldHandle* field, caf::UiEditorAttribute* attribute) override;
};
