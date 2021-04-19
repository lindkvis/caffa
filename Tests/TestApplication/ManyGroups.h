#pragma once

#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafObject.h"

namespace caffa
{
class Variant;
}

class ManyGroups : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    ManyGroups();

    caffa::Field<double>      m_doubleField;
    caffa::Field<int>         m_intField;
    caffa::Field<std::string> m_textField;
    caffa::Field<double>      m_proxyDoubleField;

    caffa::Field<std::vector<std::string>> m_multiSelectList;
    caffa::Field<std::string>              m_stringWithMultipleOptions;

    caffa::Field<bool>  m_toggleField;
    caffa::FieldHandle* objectToggleField() override;

    void onFieldChangedByCapability(const caffa::FieldHandle*     changedField,
                                    const caffa::FieldCapability* changedFieldCapability,
                                    const caffa::Variant&         oldValue,
                                    const caffa::Variant&         newValue) override;

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
    std::deque<caffa::OptionItemInfo> calculateValueOptions(const caffa::FieldHandle* fieldNeedingOptions,
                                                          bool*                   useOptionsOnly) override;

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(caffa::UiOrdering& uiOrdering) override;

    void defineEditorAttribute(const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute) override;
};
