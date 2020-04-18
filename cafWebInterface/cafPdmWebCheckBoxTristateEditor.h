#pragma once

#include "cafPdmWebFieldEditorHandle.h"

#include <Wt/Core/observing_ptr.hpp>
#include <Wt/WWidget.h>
#include <Wt/WCheckBox.h>
#include <Wt/WLabel.h>

namespace caf 
{

class PdmWebCheckBoxTristateEditor : public PdmWebFieldEditorHandle
{
    CAF_PDM_WEB_FIELD_EDITOR_HEADER_INIT;

public:
    PdmWebCheckBoxTristateEditor()          {} 
    ~PdmWebCheckBoxTristateEditor() override {} 

protected:
    Wt::WWidget*    createEditorWidget() override;
    Wt::WLabel* createLabelWidget() override;
    void            configureAndUpdateUi(const QString& uiConfigName) override;

protected:
    void            slotClicked();

private:
    Wt::Core::observing_ptr<Wt::WCheckBox> m_checkBox;
    Wt::Core::observing_ptr<Wt::WLabel >   m_label;
};


} // end namespace caf
