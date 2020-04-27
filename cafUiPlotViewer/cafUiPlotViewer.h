#pragma once

#include "cafPlotViewerInterface.h"

#include <QPointer>

#include "qwt_plot.h"

namespace caf
{
class UiPlotViewer : public PlotViewerInterface
{
public:
    UiPlotViewer();
    virtual ~UiPlotViewer();

    QwtPlot* getOrCreateViewer();

private:
    static QwtPlot::Axis axis( Axis axisEnum, Orientation orientation );

    void dataChanged( void* changedVariable = nullptr ) override;
    void assignPropertiesToAxis( Axis axisEnum, const AxisProperties& properties ) override;

private:
    QPointer<QwtPlot> m_viewer;
};
} // namespace caf
