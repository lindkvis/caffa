

#pragma once
#include "cafUiFieldEditorHandle.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QWidget>

namespace caf
{
class PdmUiCheckBoxTristateEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxTristateEditor() {}
    ~PdmUiCheckBoxTristateEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;

protected slots:
    void slotClicked( bool );

private:
    QPointer<QCheckBox>       m_checkBox;
    QPointer<QLabel> m_label;
};

} // end namespace caf
