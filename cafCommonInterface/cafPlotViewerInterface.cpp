#include "cafPlotViewerInterface.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::PlotViewerInterface()
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
PlotViewerInterface::~PlotViewerInterface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setTitle( const QString& title )
{
    m_title = title;
    dataChanged( &m_title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PlotViewerInterface::title() const
{
    return m_title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setTitleEnabled( bool enabled )
{
    m_titleEnabled = enabled;
    dataChanged( &m_titleEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PlotViewerInterface::titleEnabled() const
{
    return m_titleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setOrientation( Orientation orientation )
{
    m_orientation = orientation;
    dataChanged( &m_orientation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::Orientation PlotViewerInterface::orientation() const
{
    return m_orientation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setBackgroundColor( const QColor& color )
{
    m_bgColor = color;
    dataChanged( &m_bgColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor PlotViewerInterface::backgroundColor() const
{
    return m_bgColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> PlotViewerInterface::axisRange( Axis axis ) const
{
    return axisProperties( axis ).range;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisRange( Axis axis, double minValue, double maxValue )
{
    axisProperties( axis ).range = std::make_pair( minValue, maxValue );
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisTitle( Axis axis, const QString& axisTitle )
{
    axisProperties( axis ).title = axisTitle;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PlotViewerInterface::axisTitle( Axis axis ) const
{
    return axisProperties( axis ).title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisTitleEnabled( Axis axis, bool enabled )
{
    axisProperties( axis ).titleEnabled = enabled;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PlotViewerInterface::axisTitleEnabled( Axis axis ) const
{
    return axisProperties( axis ).titleEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisInverted( Axis axis, bool inverted )
{
    axisProperties( axis ).inverted = inverted;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisLabelsAndTicksEnabled( Axis axis, bool enableLabels, bool enableTicks )
{
    axisProperties( axis ).labelsEnabled = enableLabels;
    axisProperties( axis ).ticksEnabled  = enableTicks;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FontTools::Size PlotViewerInterface::axisTitleFontPointSize( Axis axis ) const
{
    return axisProperties( axis ).titleFontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FontTools::Size PlotViewerInterface::axisValueFontPointSize( Axis axis ) const
{
    return axisProperties( axis ).valueFontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setAxisFontsAndAlignment( Axis            axis,
                                                    FontTools::Size titleFontSize,
                                                    FontTools::Size valueFontSize,
                                                    bool            titleBold /*= false*/,
                                                    int             alignment /*= (int)Qt::AlignRight */ )
{
    AxisProperties& props = axisProperties( axis );
    props.titleFontSize   = titleFontSize;
    props.valueFontSize   = valueFontSize;
    props.titleBold       = titleBold;
    props.titleAlignment  = alignment;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::enableGridLines( Axis axis, bool majorGridLines, bool minorGridLines )
{
    axisProperties( axis ).majorGridLines = majorGridLines;
    axisProperties( axis ).minorGridLines = minorGridLines;
    dataChanged( &m_axisProperties );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setLegendFontPointSize( int fontSizePt )
{
    m_legendFontSizePt = fontSizePt;
    dataChanged( &m_legendFontSizePt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setLegendEnabled( bool enabled )
{
    m_legendEnabled = enabled;
    dataChanged( &m_legendEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PlotViewerInterface::legendEnabled() const
{
    return m_legendEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setZoomEnabled( bool enabled )
{
    m_zoomEnabled = enabled;
    dataChanged( &m_zoomEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PlotViewerInterface::zoomEnabled() const
{
    return m_zoomEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::setPanEnabled( bool enabled )
{
    m_panEnabled = enabled;
    dataChanged( &m_panEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PlotViewerInterface::panEnabled() const
{
    return m_panEnabled;
}

//--------------------------------------------------------------------------------------------------
/// Takes ownership of the curve
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::addCurve( std::shared_ptr<CurveInterface> curveToAdd )
{
    m_curves.push_back( curveToAdd );
    curveToAdd->attachToPlot( this );
    dataChanged( &m_curves );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::removeCurve( CurveInterface* curveToRemove )
{
    curveToRemove->detachFromPlot();
    m_curves.remove_if( [&]( std::shared_ptr<CurveInterface> curve ) { return curveToRemove == curve.get(); } );
    dataChanged( &m_curves );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PlotViewerInterface::removeAllCurves()
{
    while ( !m_curves.empty() )
    {
        removeCurve( m_curves.back().get() );
    }
    dataChanged( &m_curves );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::shared_ptr<CurveInterface>> PlotViewerInterface::curves() const
{
    return m_curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::AxisProperties& PlotViewerInterface::axisProperties( Axis axis )
{
    return m_axisProperties[axis];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::AxisProperties PlotViewerInterface::axisProperties( Axis axis ) const
{
    auto it = m_axisProperties.find( axis );
    return it != m_axisProperties.end() ? it->second : AxisProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PlotViewerInterface::AxisProperties::AxisProperties()
    : title( "axis" )
    , titleEnabled( true )
    , titleBold( false )
    , titleAlignment( Qt::AlignRight )
    , titleFontSize( FontTools::Size::Medium )
    , valueFontSize( FontTools::Size::XSmall )
    , inverted( false )
    , labelsEnabled( true )
    , ticksEnabled( true )
    , labelAngleDegrees( 0.0 )
    , majorGridLines( false )
    , minorGridLines( false )
    , autoScale( true )
    , range( 0.0, 0.0 )
{
}
