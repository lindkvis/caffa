#include "cafWebPlotViewer.h"

#include "cafWebPlotCurve.h"

#include <Wt/Chart/WAxis.h>
#include <Wt/Chart/WCartesianChart.h>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WebPlotViewer::WebPlotViewer()
    : m_title( "Plot" )
    , m_orientation( Orientation::Vertical )
    , m_bgColor( Qt::white )
    , m_legendEnabled( true )
    , m_zoomEnabled( false )
    , m_panEnabled( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WebPlotViewer::~WebPlotViewer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Chart::WCartesianChart* WebPlotViewer::getOrCreateChart()
{
    if ( !m_chart )
    {
        m_chart = new Wt::Chart::WCartesianChart;
        m_chart->setType( Wt::Chart::ChartType::Scatter ); // Set type to ScatterPlot.
        m_chart->setMargin( 10, Wt::Side::Top | Wt::Side::Bottom ); // Add margin vertically
        m_chart->setMargin( Wt::WLength::Auto, Wt::Side::Left | Wt::Side::Right ); // Center horizontally
        updatePlot();
    }
    return m_chart.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setTitle( const QString& title )
{
    m_title = title;
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString WebPlotViewer::title() const
{
    return m_title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setOrientation( Orientation orientation )
{
    m_orientation = orientation;
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::Orientation WebPlotViewer::orientation() const
{
    return m_orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setBackgroundColor( const QColor& color )
{
    m_bgColor = color;
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor WebPlotViewer::backgroundColor() const
{
    return m_bgColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> WebPlotViewer::axisRange( Axis axis ) const
{
    auto it = m_axisRanges.find( axis );
    return it != m_axisRanges.end() ? it->second : std::make_pair( 0.0, 0.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setAxisRange( Axis axis, double minValue, double maxValue )
{
    m_axisRanges[axis] = std::make_pair( minValue, maxValue );
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setAxisTitle( Axis axis, const QString& axisTitle )
{
    this->axis( axis ).setTitle( axisTitle.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString WebPlotViewer::axisTitle( Axis axis ) const
{
    return QString::fromStdString( this->axis( axis ).title().narrow() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setLegendEnabled( bool enabled )
{
    m_legendEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WebPlotViewer::legendEnabled() const
{
    return m_legendEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setZoomEnabled( bool enabled )
{
    m_zoomEnabled = enabled;
    updatePlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WebPlotViewer::zoomEnabled() const
{
    return m_zoomEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::setPanEnabled( bool enabled )
{
    m_panEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WebPlotViewer::panEnabled() const
{
    return m_panEnabled;
}

//--------------------------------------------------------------------------------------------------
/// Takes ownership of the curve
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::addCurve( std::shared_ptr<CurveInterface> curveToAdd )
{
    m_curves.push_back( curveToAdd );
    curveToAdd->attachToPlot( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::removeCurve( CurveInterface* curveToRemove )
{
    curveToRemove->detachFromPlot();
    m_curves.remove_if( [&]( std::shared_ptr<CurveInterface> curve ) { return curveToRemove == curve.get(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::addSeries( CurveInterface* curve, std::unique_ptr<Wt::Chart::WDataSeries> dataSeries )
{
    m_chart->addSeries( std::move( dataSeries ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::removeSeries( Wt::Chart::WDataSeries* dataSeries )
{
    m_chart->removeSeries( dataSeries );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::removeAllCurves()
{
    while ( !m_curves.empty() )
    {
        removeCurve( m_curves.back().get() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::shared_ptr<CurveInterface>> WebPlotViewer::curves() const
{
    return m_curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::updatePlot()
{
    if ( m_chart )
    {
        if ( !m_title.isEmpty() )
        {
            m_chart->setPlotAreaPadding( 60, Wt::Side::Top );
            m_chart->setTitle( m_title.toStdString() );
        }
        else
        {
            m_chart->setPlotAreaPadding( 10, Wt::Side::Top );
            m_chart->setTitle( "" );
        }

        m_chart->axis( Wt::Chart::Axis::X ).setLocation( Wt::Chart::AxisValue::Minimum );
        m_chart->axis( Wt::Chart::Axis::Y ).setLocation( Wt::Chart::AxisValue::Minimum );

        m_chart->setPlotAreaPadding( 60, Wt::Side::Left );

        // The CAF orientation is opposite to that of Wt
        if ( m_orientation == Orientation::Vertical )
        {
            m_chart->setOrientation( Wt::Orientation::Horizontal );
            m_chart->setPlotAreaPadding( m_chart->plotAreaPadding( Wt::Side::Top ) + 40, Wt::Side::Top );
        }
        else
        {
            m_chart->setOrientation( Wt::Orientation::Vertical );
            m_chart->setPlotAreaPadding( m_chart->plotAreaPadding( Wt::Side::Bottom ) + 40, Wt::Side::Bottom );
        }
        // Legend padding
        m_chart->setPlotAreaPadding( 120, Wt::Side::Right );

        m_chart->setBackground( Wt::WBrush( Wt::WColor( m_bgColor.red(), m_bgColor.green(), m_bgColor.blue() ) ) );
        for ( auto axisRangePair : m_axisRanges )
        {
            this->axis( axisRangePair.first ).setRange( axisRangePair.second.first, axisRangePair.second.second );
        }
        m_chart->setLegendEnabled( m_legendEnabled );
        m_chart->setZoomEnabled( m_zoomEnabled );
        m_chart->setPanEnabled( m_panEnabled );
        m_chart->refresh();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Chart::WAxis& WebPlotViewer::axis( Axis axis ) const
{
    CAF_ASSERT( m_chart );
    switch ( axis )
    {
        case PlotViewerInterface::Axis::yLeft:
            return m_chart->axis( Wt::Chart::Axis::Y1 );
        case PlotViewerInterface::Axis::yRight:
            return m_chart->axis( Wt::Chart::Axis::Y2 );
        case PlotViewerInterface::Axis::x:
        {
            return m_chart->axis( Wt::Chart::Axis::X );
        }
        default:
            break;
    }
    CAF_ASSERT( "Axis does not exist" );
    return m_chart->axis( Wt::Chart::Axis::X );
}
