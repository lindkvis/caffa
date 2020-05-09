#pragma once

#include "cafPlotViewerInterface.h"
#include "cafWebPlotCurve.h"

#include <list>
#include <map>
#include <memory>
#include <utility>

#include <Wt/Core/observing_ptr.hpp>

namespace Wt
{
namespace Chart
{
    class WAbstractChartModel;
    class WAxis;
    class WCartesianChart;
} // namespace Chart
} // namespace Wt

namespace caf
{
class WebPlotViewer : public PlotViewerInterface
{
public:
    WebPlotViewer();
    virtual ~WebPlotViewer();
    Wt::Chart::WCartesianChart* getOrCreateViewer();

private:
    friend class WebPlotCurve;

    Wt::Chart::WAxis& axis( Axis axis ) const;
    void              addSeries( CurveInterface* curve, std::unique_ptr<Wt::Chart::WDataSeries> dataSeries );
    void              removeSeries( Wt::Chart::WDataSeries* dataSeries );
    void              dataChanged( void* changedVariable = nullptr ) override;
    Wt::FontSize      convertFontSize( FontTools::DeltaSize size );
    void              assignPropertiesToAxis( Axis axisEnum, const AxisProperties& properties );

private:
    Wt::Core::observing_ptr<Wt::Chart::WCartesianChart> m_viewer;
};

} // namespace caf
