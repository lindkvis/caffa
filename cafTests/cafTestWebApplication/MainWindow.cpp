#include "MainWindow.h"

#include "cafAppEnum.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"
#include "cafCmdSelectionHelper.h"
#include "cafColor3.h"

#include "WPopupMenuWrapper.h"
#include "cafFilePath.h"
#include "cafPdmDocument.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebSliderEditor.h"
#include "cafPdmXmlColor3f.h"
#include "cafSelectionManager.h"
#include "cafWebPlotCurve.h"
#include "cafWebPlotViewer.h"

#include "Wt/Chart/WCartesianChart.h"
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTimer.h>
#include <Wt/WVBoxLayout.h>

#include <QDate>
#include <QString>

#include <cmath>
#include <functional>
#include <memory>

using namespace std::placeholders;

class DemoPdmObjectGroup : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObjectGroup()
    {
        CAF_PDM_InitFieldNoDefault(&objects, "PdmObjects", "", "", "", "")

            objects.uiCapability()
                ->setUiHidden(true);
    }

public:
    caf::PdmChildArrayField<PdmObjectHandle*> objects;
};

CAF_PDM_SOURCE_INIT(DemoPdmObjectGroup, "DemoPdmObjectGroup");

class SmallDemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    SmallDemoPdmObject()
    {
        CAF_PDM_InitObject("Small Demo Object  with a very long title",
                           ":/images/win/filenew.png",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField,
                          "BigNumber",
                          0.123,
                          "Big Number",
                          "",
                          "Enter a big number here",
                          "This is a place you can enter a big real value if you want");
        m_doubleField.uiCapability()->setCustomContextMenuEnabled(true);

        CAF_PDM_InitField(&m_intField,
                          "IntNumber",
                          3,
                          "Small Number",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        m_intField.uiCapability()->setUiEditorTypeName(caf::PdmWebSliderEditor::uiEditorTypeName());
        CAF_PDM_InitField(&m_textField,
                          "TextField",
                          QString("A cow jumped over whatever"),
                          "Text",
                          "",
                          "Text tooltip",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitFieldNoDefault(&m_testEnumField, "TestEnumValue", "EnumField", "", "", "");

        m_proxyDoubleField.registerSetMethod(this, &SmallDemoPdmObject::setDoubleMember);
        m_proxyDoubleField.registerGetMethod(this, &SmallDemoPdmObject::doubleMember);
        CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

        m_proxyDoubleField = 0;
    }

    caf::PdmField<double>                     m_doubleField;
    caf::PdmField<int>                        m_intField;
    caf::PdmField<QString>                    m_textField;
    caf::PdmField<bool>                       m_toggleField;
    caf::PdmField<caf::AppEnum<TestEnumType>> m_testEnumField;

    caf::PdmProxyValueField<double> m_proxyDoubleField;

    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if (changedField == &m_doubleField)
        {
            if (MainWindow::instance()) MainWindow::instance()->debug(QString("Double Field changed to %1").arg(m_doubleField()));
        }
        else if (changedField == &m_intField)
        {
            if (MainWindow::instance()) MainWindow::instance()->debug(QString("Int Field changed to %1").arg(m_intField()));
        }
        else if (changedField == &m_textField)
        {
            if (MainWindow::instance()) MainWindow::instance()->debug(QString("Text Field changed to %1").arg(m_textField()));
        }
        else if (changedField == &m_toggleField)
        {
            if (MainWindow::instance())
                MainWindow::instance()->debug(QString("Toggle Field changed to %1").arg(m_toggleField() ? "TRUE" : "FALSE"));
        }
        else if (changedField == &m_testEnumField)
        {
            if (MainWindow::instance())
                MainWindow::instance()->debug(QString("Enum field changed to %1").arg(m_testEnumField().uiText()));
        }
    }

    void setDoubleMember(const double& d)
    {
        m_doubleMember = d;
        if (MainWindow::instance()) MainWindow::instance()->debug(QString("setDoubleMember(%1)").arg(d));
    }
    double doubleMember() const
    {
        return m_doubleMember;
    }

    caf::PdmFieldHandle* objectToggleField() override
    {
        return &m_toggleField;
    }

    caf::PdmFieldHandle* userDescriptionField() override
    {
        return &m_textField;
    }

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_toggleField);
        uiOrdering.add(&m_doubleField);
        uiOrdering.add(&m_intField);
        QString dynamicGroupName = QString("Dynamic Group Text (%1)").arg(m_intField);

        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword(dynamicGroupName, "MyTest");
        group->add(&m_textField);
        group->add(&m_proxyDoubleField);
        group->add(&m_testEnumField);
        uiOrdering.skipRemainingFields(true);
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override
    {
        if (field == &m_intField)
        {
            auto sliderAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>(attribute);
            if (sliderAttr)
            {
                sliderAttr->m_minimum = 1;
                sliderAttr->m_maximum = 7;
            }
        }
    }
};

CAF_PDM_SOURCE_INIT(SmallDemoPdmObject, "SmallDemoPdmObject");

class ColorAndDateEditorPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ColorAndDateEditorPdmObject()
    {
        CAF_PDM_InitObject("Color And Date and File Upload Editor Object",
                           "",
                           "This object is a demo of the CAF framework",
                           "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_intFieldStandard,
                          "Standard",
                          0,
                          "Fairly Wide Label",
                          "",
                          "Enter some small number here",
                          "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_colorField, "ColorField", caf::Color3f(0.2, 0.7, 0.9), "Color Field", "", "Click to set color", "");
        CAF_PDM_InitField(&m_dateField, "DateField", QDate(2012, 05, 19), "Date Field", "", "Set a date", "");
        CAF_PDM_InitFieldNoDefault(
            &m_fileUploadField, "FileUploadField", "Upload File", "", "This editor lets you upload a file", "");
    }

    // Outside group
    caf::PdmField<int>           m_intFieldStandard;
    caf::PdmField<caf::Color3f>  m_colorField;
    caf::PdmField<QDate>         m_dateField;
    caf::PdmField<caf::FilePath> m_fileUploadField;

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_intFieldStandard);
        // uiOrdering.add(&m_colorField);
        uiOrdering.add(&m_dateField);
        uiOrdering.add(&m_fileUploadField);
        uiOrdering.skipRemainingFields(true);
    }
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override
    {
        if (changedField == &m_colorField)
        {
            if (MainWindow::instance())
                MainWindow::instance()->debug(QString("Color changed to:  %1,%2,%3")
                                                  .arg(m_colorField().r())
                                                  .arg(m_colorField().g())
                                                  .arg(m_colorField().b()));
        }
        else if (changedField == &m_dateField)
        {
            if (MainWindow::instance())
                MainWindow::instance()->debug(QString("Date changed to: %1").arg(m_dateField().toString(Qt::ISODate)));
        }
        else if (changedField == &m_fileUploadField)
        {
            if (MainWindow::instance())
                MainWindow::instance()->debug(QString("File uploaded to: %1").arg(m_fileUploadField().path()));
        }
    }
};

CAF_PDM_SOURCE_INIT(ColorAndDateEditorPdmObject, "ColorAndDateEditorPdmObject");

namespace caf
{
template<>
void AppEnum<SmallDemoPdmObject::TestEnumType>::setUp()
{
    addItem(SmallDemoPdmObject::T1, "T1", "An A letter");
    addItem(SmallDemoPdmObject::T2, "T2", "A B letter");
    addItem(SmallDemoPdmObject::T3, "T3", "A B C letter");
    setDefault(SmallDemoPdmObject::T1);
}
} // namespace caf

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WPanel> MainWindow::createPlotPanel()
{
    m_plot.reset(new caf::WebPlotViewer);
    m_plot->getOrCreateViewer();
    auto container = std::make_unique<WContainerWidget>();

    {
        std::vector<double> xValues;
        std::vector<double> yValues;

        for (unsigned i = 0; i < 400; ++i)
        {
            double x = (static_cast<double>(i)) / 40;
            double y = std::sin(x) + 1;
            xValues.push_back(x);
            yValues.push_back(y);
        }
        auto curve = std::make_shared<caf::WebPlotCurve>();
        curve->setSamplesFromXValuesAndYValues(xValues, yValues);
        curve->setName("y = sin(x)");
        curve->setColor(QColor(Qt::blue));
        m_plot->addCurve(curve);
    }
    {
        std::vector<double> xValues;
        std::vector<double> yValues;

        for (unsigned i = 0; i < 400; ++i)
        {
            double x = (static_cast<double>(i)) / 40;
            double y = std::cos(x) + 1;
            xValues.push_back(x);
            yValues.push_back(y);
        }
        auto curve = std::make_shared<caf::WebPlotCurve>();
        curve->setSamplesFromXValuesAndYValues(xValues, yValues);
        curve->setName("y = cos(x)");
        curve->setColor(QColor(Qt::red));
        m_plot->addCurve(curve);
    }

    /*
     * Create the scatter plot.
     */
    m_plot->setLegendEnabled(true); // Enable the legend.
    m_plot->setOrientation(caf::PlotViewerInterface::Orientation::Vertical);
    m_plot->setAxisTitle(caf::PlotViewerInterface::Axis::x, "Depth");
    m_plot->setAxisTitle(caf::PlotViewerInterface::Axis::yLeft, "Something");
    Wt::Chart::WCartesianChart* chart =
        container->addWidget(std::unique_ptr<Wt::Chart::WCartesianChart>(m_plot->getOrCreateViewer()));

    // Add the curves

    chart->resize(600, 600); // WPaintedWidget must be given explicit size.

    auto panel = std::make_unique<Wt::WPanel>();
    panel->setCentralWidget(std::move(container));
    panel->setTitle("Plot");
    return panel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
    : m_debugWindow(nullptr)
{
    sm_mainWindowInstance = this;

    setTitle("Test Application");

    buildTestModel();

    m_pdmUiTreeView = new caf::PdmWebTreeView;
    m_pdmUiTreeView->treeView()->setSelectionMode(Wt::SelectionMode::Extended);
    m_pdmUiTreeView->treeView()->setMinimumSize(150, 200);
    m_pdmUiTreeView->enableSelectionManagerUpdating(true);
    m_pdmUiTreeView->enableDefaultContextMenu(false);
    m_pdmUiTreeView->treeView()->mouseWentUp().connect(std::bind(&MainWindow::slotCustomMenuRequested, this, _1, _2));
    m_pdmUiTreeView->treeView()->setRootIsDecorated(false);

    m_objectEditor = new caf::PdmWebDefaultObjectEditor;

    setPdmRoot(m_testRoot);
    std::unique_ptr<Wt::WContainerWidget> objectWidget(m_objectEditor->getOrCreateWidget());

    caf::PdmUiItem::enableExtraDebugText(true);
    this->setPadding(0);
    auto windowLayout = std::make_unique<Wt::WHBoxLayout>();
    windowLayout->setContentsMargins(0, 0, 0, 0);
    auto leftColumnLayout = std::make_unique<Wt::WVBoxLayout>();
    leftColumnLayout->setContentsMargins(2, 2, 2, 2);
    auto centreColumnLayout = std::make_unique<Wt::WVBoxLayout>();
    centreColumnLayout->setContentsMargins(2, 2, 2, 2);
    auto rightColumnLayout = std::make_unique<Wt::WVBoxLayout>();
    rightColumnLayout->setContentsMargins(2, 2, 2, 2);

    auto plotPanel = createPlotPanel();

    auto debugPanel = std::make_unique<Wt::WPanel>();
    debugPanel->setTitle("Debug Information");
    debugPanel->setStyleClass("panel-tight");
    debugPanel->setMaximumSize(Wt::WLength::Auto, Wt::WLength(50, Wt::LengthUnit::FontEx));
    auto debugContainer = std::make_unique<Wt::WContainerWidget>();
    debugContainer->setLayout(std::make_unique<Wt::WVBoxLayout>());
    auto debugWindow = std::make_unique<Wt::WTextArea>();
    m_debugWindow    = debugWindow.get();
    debugContainer->layout()->addWidget(std::move(debugWindow));
    debugPanel->setCentralWidget(std::move(debugContainer));

    leftColumnLayout->addWidget(std::unique_ptr<caf::PdmWebTreeView>(m_pdmUiTreeView), 1);

    centreColumnLayout->addWidget(std::move(plotPanel), 2);
    centreColumnLayout->addWidget(std::move(debugPanel), 0);
    rightColumnLayout->addWidget(std::move(objectWidget), 5);
    rightColumnLayout->addWidget(std::make_unique<Wt::WContainerWidget>(), 0);
    // auto progressBar = caf::WebProgressBar::instance();
    // rightColumnLayout->addWidget(std::unique_ptr<caf::WebProgressBar>(progressBar.get()));

    windowLayout->addLayout(std::move(leftColumnLayout), 1);
    windowLayout->addLayout(std::move(centreColumnLayout), 6);
    windowLayout->addLayout(std::move(rightColumnLayout), 1);

    windowLayout->setResizable(0);
    windowLayout->setResizable(1);
    windowLayout->setResizable(2);

    this->setLayout(std::move(windowLayout));

    std::vector<caf::PdmObject*> objects;
    m_testRoot->descendantsIncludingThisOfType(objects);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow* MainWindow::instance()
{
    return sm_mainWindowInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setPdmRoot(caf::PdmObjectHandle* pdmRoot)
{
    caf::PdmUiObjectHandle* uiObject = uiObj(pdmRoot);

    m_pdmUiTreeView->setPdmItem(uiObject);
    m_pdmUiTreeView->selectionChanged().connect(this, &MainWindow::slotSimpleSelectionChanged);

    std::vector<SmallDemoPdmObject*> obj;
    if (pdmRoot)
    {
        pdmRoot->descendantsIncludingThisOfType(obj);
    }

    m_objectEditor->setPdmObject(obj[0]);

    caf::SelectionManager::instance()->setPdmRootObject(m_testRoot);
    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::debug(const QString& string)
{
    if (m_debugWindow)
    {
        m_debugWindow->setText(m_debugWindow->text() + string.toStdString() + "\n");
        m_debugWindow->doJavaScript(m_debugWindow->jsRef() + ".scrollTop = " + m_debugWindow->jsRef() + ".scrollHeight;");
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::buildTestModel()
{
    m_testRoot = new DemoPdmObjectGroup;

    SmallDemoPdmObject* smallObj1 = new SmallDemoPdmObject;
    m_testRoot->objects.push_back(smallObj1);

    SmallDemoPdmObject* smallObj2 = new SmallDemoPdmObject;
    m_testRoot->objects.push_back(smallObj2);
    SmallDemoPdmObject* smallObj3 = new SmallDemoPdmObject;
    m_testRoot->objects.push_back(smallObj3);
    ColorAndDateEditorPdmObject* smallObj4 = new ColorAndDateEditorPdmObject;
    m_testRoot->objects.push_back(smallObj4);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSimpleSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);
    caf::PdmObjectHandle*          obj       = nullptr;
    caf::PdmChildArrayFieldHandle* listField = nullptr;

    if (selection.size())
    {
        caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>(selection[0]);
        if (pdmUiObj) obj = pdmUiObj->objectHandle();
    }

    m_objectEditor->setPdmObject(obj);
    m_objectEditor->updateUi();
    wApp->triggerUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotCustomMenuRequested(const Wt::WModelIndex& item, const Wt::WMouseEvent& event)
{
    if (event.button() == Wt::MouseButton::Right)
    {
        if (!m_pdmUiTreeView->treeView()->isSelected(item))
        {
            m_pdmUiTreeView->treeView()->select(item);
            // caf::SelectionManager::instance()->setSelectedItem(m_pdmUiTreeView->uiItemFromModelIndex(item));
        }

        m_popup = std::make_unique<caf::WPopupMenuWrapper>();
        m_popup->menu()->aboutToHide().connect(std::bind(&MainWindow::slotOnActionSelection, this));

        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "cafToggleItemsOnFeature";
        menuBuilder << "cafToggleItemsOffFeature";
        menuBuilder << "cafToggleItemsFeature";
        menuBuilder << "Separator";
        menuBuilder << "cafToggleItemsOnOthersOffFeature";
        menuBuilder.appendToMenu(m_popup.get());

        m_popup->refreshEnabledState();
        if (m_popup->menu()->isHidden())
            m_popup->menu()->popup(event);
        else
            m_popup->menu()->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotOnActionSelection()
{
    if (m_popup->menu()->result())
    {
        Wt::WString                         text   = m_popup->menu()->result()->text();
        std::shared_ptr<caf::ActionWrapper> action = m_popup->findAction(QString::fromStdString(text.narrow()));
        action->trigger(m_popup->menu()->result()->isChecked());
    }
}
