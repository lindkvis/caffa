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
    ~WebPlotViewer();
    Wt::Chart::WCartesianChart* getOrCreateChart();

    // Plot Interface overrides
    void    setTitle( const QString& title ) override;
    QString title() const override;

    void        setOrientation( Orientation orientation ) override;
    Orientation orientation() const override;

    void   setBackgroundColor( const QColor& color ) override;
    QColor backgroundColor() const override;

    std::pair<double, double> axisRange( Axis axis ) const override;
    void                      setAxisRange( Axis axis, double minValue, double maxValue ) override;
    void                      setAxisTitle( Axis axis, const QString& axisTitle ) override;
    QString                   axisTitle( Axis axis ) const override;

    void setLegendEnabled( bool enabled ) override;
    bool legendEnabled() const override;
    void setZoomEnabled( bool enabled ) override;
    bool zoomEnabled() const override;
    void setPanEnabled( bool enabled ) override;
    bool panEnabled() const override;

    void addCurve( std::shared_ptr<CurveInterface> curveToAdd ) override;
    void removeCurve( CurveInterface* curve ) override;
    void removeAllCurves() override;

    std::list<std::shared_ptr<CurveInterface>> curves() const;

    void updatePlot() override;

private:
    friend class WebPlotCurve;

    Wt::Chart::WAxis& axis( Axis axis ) const;
    void              addSeries( CurveInterface* curve, std::unique_ptr<Wt::Chart::WDataSeries> dataSeries );
    void              removeSeries( Wt::Chart::WDataSeries* dataSeries );

private:
    Wt::Core::observing_ptr<Wt::Chart::WCartesianChart> m_chart;
    std::list<std::shared_ptr<CurveInterface>>          m_curves;

    QString                                   m_title;
    Orientation                               m_orientation;
    QColor                                    m_bgColor;
    std::map<Axis, std::pair<double, double>> m_axisRanges;
    std::map<Axis, QString>                   m_axisTitles;
    bool                                      m_legendEnabled;
    bool                                      m_zoomEnabled;
    bool                                      m_panEnabled;
};

} // namespace caf
