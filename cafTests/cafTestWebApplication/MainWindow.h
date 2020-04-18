#pragma once

#include "WPopupMenuWrapper.h"
#include "cafPdmWebTreeView.h"

#include "Wt/Core/observing_ptr.hpp"
#include "Wt/WGroupBox.h"
#include "Wt/WHBoxLayout.h"
#include "Wt/WPanel.h"
#include "Wt/WProgressBar.h"
#include "Wt/WTextArea.h"

class QString;
class DemoPdmObjectGroup;

namespace caf
{
class PdmDocument;
class PdmObjectCollection;
class PdmObjectHandle;
class PdmWebDefaultObjectEditor;
class WebPlotViewer;
} // namespace caf

class MainWindow : public Wt::WGroupBox
{
public:
    MainWindow();
    ~MainWindow() override;

    static MainWindow* instance();
    void               setPdmRoot(caf::PdmObjectHandle* pdmRoot);
    void               debug(const QString& string);

private:
    void               buildTestModel();
    void               slotSimpleSelectionChanged();
    void               slotCustomMenuRequested(const Wt::WModelIndex& item, const Wt::WMouseEvent& event);
    void               slotOnActionSelection();
    static MainWindow* sm_mainWindowInstance;

    std::unique_ptr<Wt::WPanel> createPlotPanel();

private:
    DemoPdmObjectGroup* m_testRoot;

    caf::PdmWebDefaultObjectEditor*         m_objectEditor;
    Wt::WTextArea*                          m_debugWindow;
    caf::PdmWebTreeView*                    m_pdmUiTreeView;
    std::unique_ptr<caf::WPopupMenuWrapper> m_popup;

    std::unique_ptr<caf::WebPlotViewer> m_plot;
};
