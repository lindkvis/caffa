#include "cafUiPlotViewer.h"

using namespace caf;

#include "qwt_plot.h"
#include "qwt_plot_grid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPlotViewer::UiPlotViewer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPlotViewer::~UiPlotViewer()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot* caf::UiPlotViewer::getOrCreateViewer()
{
    if ( !m_viewer )
    {
        m_viewer = new QwtPlot;
    }
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiPlotViewer::dataChanged( void* changedVariable )
{
    if ( m_viewer )
    {
        if ( !m_title.isEmpty() && m_titleEnabled )
        {
            m_viewer->setTitle( m_title );
        }
        else
        {
            m_viewer->setTitle( "" );
        }

        m_viewer->setCanvasBackground( QBrush( QColor( m_bgColor.red(), m_bgColor.green(), m_bgColor.blue() ) ) );

        for ( auto axisRangePair : m_axisProperties )
        {
            assignPropertiesToAxis( axisRangePair.first, axisRangePair.second );
        }

        /* m_viewer->setLegendEnabled( m_legendEnabled );
        m_viewer->setZoomEnabled( m_zoomEnabled );
        m_viewer->setPanEnabled( m_panEnabled );
        m_viewer->refresh();*/

        m_viewer->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot::Axis caf::UiPlotViewer::axis( Axis axisEnum, Orientation orientation )
{
    switch ( axisEnum )
    {
        case PlotViewerInterface::Axis::yLeft:
            return QwtPlot::yLeft;
        case PlotViewerInterface::Axis::yRight:
            return QwtPlot::yRight;
        case PlotViewerInterface::Axis::x:
        {
            if ( orientation == Orientation::Vertical )
            {
                return QwtPlot::xTop;
            }
            return QwtPlot::xBottom;
        }
        default:
            break;
    }
    return QwtPlot::xBottom;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiPlotViewer::assignPropertiesToAxis( Axis axisEnum, const AxisProperties& properties )
{
    QwtPlot::Axis axis = this->axis( axisEnum, m_orientation );

    if ( m_viewer )
    {
        if ( properties.autoScale )
        {
            m_viewer->setAxisAutoScale( axis, true );
        }
        else
        {
            m_viewer->setAxisScale( axis, properties.range.first, properties.range.second );
        }
        m_viewer->setAxisTitle( axis, properties.titleEnabled ? properties.title : "" );

        QwtPlotItemList plotItems = m_viewer->itemList( QwtPlotItem::Rtti_PlotGrid );
        for ( QwtPlotItem* plotItem : plotItems )
        {
            QwtPlotGrid* grid = static_cast<QwtPlotGrid*>( plotItem );
            if ( axis == QwtPlot::xTop || axis == QwtPlot::xBottom )
            {
                grid->setXAxis( axis );
                grid->enableX( properties.majorGridLines );
                grid->enableXMin( properties.minorGridLines );
            }
            else
            {
                grid->setYAxis( axis );
                grid->enableY( properties.majorGridLines );
                grid->enableYMin( properties.minorGridLines );
            }
            grid->setMajorPen( Qt::lightGray, 1.0, Qt::SolidLine );
            grid->setMinorPen( Qt::lightGray, 1.0, Qt::DashLine );
        }
    }
}
