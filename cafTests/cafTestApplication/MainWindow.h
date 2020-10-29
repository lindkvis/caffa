#pragma once

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QMainWindow>

class DemoObject;
class DemoObjectGroup;
class QTreeView;
class QUndoView;
class QLabel;

namespace caf
{
class ObjectCollection;
class ObjectHandle;
class UiTreeModelPdm;
class PdmUiPropertyView;
class PdmUiTreeView;
class PdmUiTableView;
class CustomObjectEditor;
} // namespace caf

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    static MainWindow* instance();
    void               setPdmRoot(caf::ObjectHandle* pdmRoot);

private:
    void createActions();
    void createDockPanels();

    void buildTestModel();
    void releaseTestData();

private slots:
    void slotInsert();
    void slotRemove();
    void slotRemoveAll();

    void slotSimpleSelectionChanged();
    void slotShowTableView();

    void slotLoadProject();
    void slotSaveProject();

    void slotCustomMenuRequestedForProjectTree(const QPoint&);

private:
    static MainWindow* sm_mainWindowInstance;

private:
    QUndoView* undoView;

    caf::PdmUiTreeView*     m_pdmUiTreeView;
    caf::PdmUiTreeView*     m_pdmUiTreeView2;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmUiTableView*    m_pdmUiTableView;
    DemoObjectGroup*     m_testRoot;

    caf::CustomObjectEditor* m_customObjectEditor;

    QLabel* m_plotLabel;
    QLabel* m_smallPlotLabel;
};
