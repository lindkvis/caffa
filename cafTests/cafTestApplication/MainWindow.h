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
class UiPropertyView;
class UiTreeView;
class UiTableView;
class CustomObjectEditor;
} // namespace caf

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    static MainWindow* instance();
    void               setRoot(caf::ObjectHandle* root);

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

    // void slotCustomMenuRequestedForProjectTree(const QPoint&);

private:
    static MainWindow* sm_mainWindowInstance;

private:
    QUndoView* undoView;

    caf::UiTreeView*     m_uiTreeView;
    caf::UiTreeView*     m_uiTreeView2;
    caf::UiPropertyView* m_uiPropertyView;
    caf::UiTableView*    m_uiTableView;
    DemoObjectGroup*     m_testRoot;

    caf::CustomObjectEditor* m_customObjectEditor;

    QLabel* m_plotLabel;
    QLabel* m_smallPlotLabel;
};
