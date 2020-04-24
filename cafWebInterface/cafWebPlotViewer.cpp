#include "cafWebPlotViewer.h"

#include "cafWebPlotCurve.h"

#include <Wt/Chart/WAxis.h>
#include <Wt/Chart/WCartesianChart.h>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WebPlotViewer::WebPlotViewer()
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
Wt::Chart::WCartesianChart* WebPlotViewer::getOrCreateViewer()
{
    if ( !m_chart )
    {
        m_chart = new Wt::Chart::WCartesianChart;
        m_chart->setType( Wt::Chart::ChartType::Scatter ); // Set type to ScatterPlot.
        m_chart->setMargin( 10, Wt::Side::Top | Wt::Side::Bottom ); // Add margin vertically
        m_chart->setMargin( Wt::WLength::Auto, Wt::Side::Left | Wt::Side::Right ); // Center horizontally
        updateViewer();
    }
    return m_chart.get();
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
void WebPlotViewer::updateViewer()
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
        for ( auto axisTitlePair : m_axisTitles )
        {
            this->axis( axisTitlePair.first ).setTitle( axisTitlePair.second.toStdString() );
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
