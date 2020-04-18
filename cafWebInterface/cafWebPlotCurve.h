#pragma once

#include "cafCurveInterface.h"

#include <Wt/Chart/WDataSeries.h>
#include <Wt/Core/observing_ptr.hpp>

#include <vector>

namespace Wt
{
namespace Chart
{
    class WDataSeries;
}
} // namespace Wt

namespace caf
{
class WebPlotViewer;
class PlotViewerInterface;

class WebPlotCurve : public CurveInterface
{
public:
    WebPlotCurve();
    ~WebPlotCurve();

    void                                    attachToPlot( PlotViewerInterface* plot ) override;
    void                                    detachFromPlot() override;
    std::unique_ptr<Wt::Chart::WDataSeries> createDataSeries() const;

    void    setName( const QString& title ) override;
    QString name() const override;

    void      setLineStyle( LineStyle lineStyle ) override;
    LineStyle lineStyle() const override;

    void   setColor( const QColor& color ) override;
    QColor color() const override;

    void setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                          const std::vector<double>& yValues,
                                          bool                       keepOnlyPositiveValues = false ) override;

    void setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                        const std::vector<double>&    yValues,
                                        bool                          keepOnlyPositiveValues = false ) override;

    void setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                        const std::vector<double>& yValues,
                                        bool                       keepOnlyPositiveValues = false ) override;

    size_t sampleCount() const override;

private:
    template <typename XDataType>
    void setSamples( const std::vector<XDataType>& xValues, const std::vector<double>& yValues, bool keepOnlyPositiveValues );

    void             applyLineStyle( Wt::Chart::WDataSeries* dataSeries ) const;
    static LineStyle getLineStyle( const Wt::Chart::WDataSeries* dataSeries );

    void          applyColor( Wt::Chart::WDataSeries* dataSeries ) const;
    static QColor getColor( const Wt::Chart::WDataSeries* dataSeries );

private:
    WebPlotViewer*                          m_plot;
    std::shared_ptr<Wt::WStandardItemModel> m_dataModel;
    Wt::Chart::WDataSeries*                 m_dataSeries;

    LineStyle m_lineStyle;
    QColor    m_color;
};

} // namespace caf
