#pragma once

#include "cafPlotViewerInterface.h"

#include <QPointer>

class QwtPlot;

namespace caf
{
class UiPlotViewer : public PlotViewerInterface
{
public:
    UiPlotViewer();
    virtual ~UiPlotViewer();

    QwtPlot* getOrCreateViewer();
    void     updateViewer() override;

private:
    QPointer<QwtPlot> m_plotWidget;
};
} // namespace caf
