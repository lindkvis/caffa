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
    if ( !m_viewer )
    {
        m_viewer = new Wt::Chart::WCartesianChart;
        m_viewer->setType( Wt::Chart::ChartType::Scatter ); // Set type to ScatterPlot.
        m_viewer->setMargin( 10, Wt::Side::Top | Wt::Side::Bottom ); // Add margin vertically
        m_viewer->setMargin( Wt::WLength::Auto, Wt::Side::Left | Wt::Side::Right ); // Center horizontally
        dataChanged( &m_viewer );
    }
    return m_viewer.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::addSeries( CurveInterface* curve, std::unique_ptr<Wt::Chart::WDataSeries> dataSeries )
{
    m_viewer->addSeries( std::move( dataSeries ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::removeSeries( Wt::Chart::WDataSeries* dataSeries )
{
    m_viewer->removeSeries( dataSeries );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotViewer::dataChanged( void* changedVariable )
{
    if ( m_viewer )
    {
        if ( !m_title.isEmpty() && m_titleEnabled )
        {
            m_viewer->setPlotAreaPadding( 60, Wt::Side::Top );
            m_viewer->setTitle( m_title.toStdString() );
        }
        else
        {
            m_viewer->setPlotAreaPadding( 10, Wt::Side::Top );
            m_viewer->setTitle( "" );
        }

        m_viewer->axis( Wt::Chart::Axis::X ).setLocation( Wt::Chart::AxisValue::Minimum );
        m_viewer->axis( Wt::Chart::Axis::Y ).setLocation( Wt::Chart::AxisValue::Minimum );

        m_viewer->setPlotAreaPadding( 60, Wt::Side::Left );

        // The CAF orientation is opposite to that of Wt
        if ( m_orientation == Orientation::Vertical )
        {
            m_viewer->setOrientation( Wt::Orientation::Horizontal );
            m_viewer->setPlotAreaPadding( m_viewer->plotAreaPadding( Wt::Side::Top ) + 40, Wt::Side::Top );
        }
        else
        {
            m_viewer->setOrientation( Wt::Orientation::Vertical );
            m_viewer->setPlotAreaPadding( m_viewer->plotAreaPadding( Wt::Side::Bottom ) + 40, Wt::Side::Bottom );
        }

        // Legend padding
        m_viewer->setPlotAreaPadding( 120, Wt::Side::Right );

        m_viewer->setBackground( Wt::WBrush( Wt::WColor( m_bgColor.red(), m_bgColor.green(), m_bgColor.blue() ) ) );
        for ( auto axisRangePair : m_axisProperties )
        {
            assignPropertiesToAxis( axisRangePair.first, axisRangePair.second );
        }

        m_viewer->setLegendEnabled( m_legendEnabled );
        m_viewer->setZoomEnabled( m_zoomEnabled );
        m_viewer->setPanEnabled( m_panEnabled );
        m_viewer->refresh();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::Chart::WAxis& WebPlotViewer::axis( Axis axis ) const
{
    CAF_ASSERT( m_viewer );
    switch ( axis )
    {
        case PlotViewerInterface::Axis::yLeft:
            return m_viewer->axis( Wt::Chart::Axis::Y1 );
        case PlotViewerInterface::Axis::yRight:
            return m_viewer->axis( Wt::Chart::Axis::Y2 );
        case PlotViewerInterface::Axis::x:
        {
            return m_viewer->axis( Wt::Chart::Axis::X );
        }
        default:
            break;
    }
    CAF_ASSERT( "Axis does not exist" );
    return m_viewer->axis( Wt::Chart::Axis::X );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::FontSize caf::WebPlotViewer::convertFontSize( FontTools::DeltaSize size )
{
    switch ( size )
    {
        case FontTools::DeltaSize::XXSmall:
            return Wt::FontSize::XXSmall;
        case FontTools::DeltaSize::XSmall:
            return Wt::FontSize::XSmall;
        case FontTools::DeltaSize::Small:
            return Wt::FontSize::Small;
        case FontTools::DeltaSize::Medium:
            return Wt::FontSize::Medium;
        case FontTools::DeltaSize::Large:
            return Wt::FontSize::Large;
        case FontTools::DeltaSize::XLarge:
            return Wt::FontSize::XLarge;
        case FontTools::DeltaSize::XXLarge:
            return Wt::FontSize::XXLarge;
        default:
            break;
    }
    return Wt::FontSize::Medium;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::WebPlotViewer::assignPropertiesToAxis( Axis axisEnum, const AxisProperties& props )
{
    Wt::Chart::WAxis& axis = this->axis( axisEnum );
    if ( props.autoScale )
    {
        axis.setAutoLimits( Wt::Chart::AxisValue::Minimum | Wt::Chart::AxisValue::Maximum );
    }
    else
    {
        axis.setRange( props.range.first, props.range.second );
    }
    axis.setTitle( props.titleEnabled ? props.title.toStdString() : "" );
    axis.setGridLinesEnabled( props.majorGridLines || props.minorGridLines );
    axis.setInverted( props.inverted );

    auto titleFont = axis.titleFont();
    titleFont.setSize( convertFontSize( props.titleFontSize ) );
    titleFont.setWeight( props.titleBold ? Wt::FontWeight::Bold : Wt::FontWeight::Normal );
    axis.setTitleFont( titleFont );

    auto labelFont = axis.labelFont();
    labelFont.setSize( convertFontSize( props.valueFontSize ) );
    axis.setLabelFont( labelFont );
}
