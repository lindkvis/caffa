#pragma once

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QMainWindow>

#include <memory>

class DemoObject;
class DemoObjectGroup;
class QTreeView;
class QUndoView;
class QLabel;

namespace caffa
{
class ObjectCollection;
class ObjectHandle;
class UiPropertyView;
class UiTreeView;
class UiTableView;
class CustomObjectEditor;
} // namespace caffa

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

    static MainWindow* instance();
    void               setRoot(caffa::ObjectHandle* root);

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

    caffa::UiTreeView*                 m_uiTreeView;
    caffa::UiTreeView*                 m_uiTreeView2;
    caffa::UiPropertyView*             m_uiPropertyView;
    caffa::UiTableView*                m_uiTableView;
    std::unique_ptr<DemoObjectGroup> m_testRoot;

    caffa::CustomObjectEditor* m_customObjectEditor;

    QLabel* m_plotLabel;
    QLabel* m_smallPlotLabel;
};
