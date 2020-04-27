#include "cafWebPlotCurve.h"

#include "cafAssert.h"
#include "cafCurveDataTools.h"
#include "cafWebPlotViewer.h"

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WStandardChartProxyModel.h>
#include <Wt/WDateTime.h>
#include <Wt/WStandardItemModel.h>

#include <algorithm>
#include <memory>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WebPlotCurve::WebPlotCurve()
    : m_plot( nullptr )
    , m_dataModel( new Wt::WStandardItemModel( 100, 2 ) )
    , m_dataSeries( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WebPlotCurve::~WebPlotCurve()
{
    this->detachFromPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::attachToPlot( PlotViewerInterface* plot )
{
    auto webPlot = dynamic_cast<WebPlotViewer*>( plot );
    CAF_ASSERT( webPlot );
    m_plot = webPlot;

    auto dataSeries = createDataSeries();
    m_dataSeries    = dataSeries.get();
    m_plot->addSeries( this, std::move( dataSeries ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::detachFromPlot()
{
    if ( m_plot )
    {
        if ( m_dataSeries )
        {
            m_plot->removeSeries( m_dataSeries );
            m_dataSeries = nullptr;
        }
        m_plot = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::Chart::WDataSeries> caf::WebPlotCurve::createDataSeries() const
{
    auto dataSeries = std::make_unique<Wt::Chart::WDataSeries>( 1, Wt::Chart::SeriesType::Line );
    dataSeries->setModel( std::make_shared<Wt::Chart::WStandardChartProxyModel>( m_dataModel ) );
    dataSeries->setXSeriesColumn( 0 );
    dataSeries->setModelColumn( 1 );
    dataSeries->setShadow( Wt::WShadow( 0.5, 0.5, Wt::WColor( 0, 0, 0, 200 ), 0.5 ) );
    applyColor( dataSeries.get() );
    applyLineStyle( dataSeries.get() );
    return dataSeries;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::WebPlotCurve::setName( const QString& title )
{
    m_dataModel->setHeaderData( 1, Wt::WString( title.toStdString() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::WebPlotCurve::name() const
{
    Wt::WString string = Wt::cpp17::any_cast<Wt::WString>( m_dataModel->headerData( 1 ) );
    return QString::fromStdString( string.narrow() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t WebPlotCurve::sampleCount() const
{
    CAF_ASSERT( m_dataModel );
    return m_dataModel->rowCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::setLineStyle( LineStyle lineStyle )
{
    m_lineStyle = lineStyle;

    if ( m_plot )
    {
        if ( m_dataSeries )
        {
            applyLineStyle( m_dataSeries );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CurveInterface::LineStyle WebPlotCurve::lineStyle() const
{
    return m_lineStyle;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::setColor( const QColor& color )
{
    m_color = color;
    if ( m_plot )
    {
        if ( m_dataSeries )
        {
            applyColor( m_dataSeries );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor WebPlotCurve::color() const
{
    return m_color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                                    const std::vector<double>& yValues,
                                                    bool                       keepOnlyPositiveValues )
{
    setSamples<double>( xValues, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                  const std::vector<double>&    yValues,
                                                  bool                          keepOnlyPositiveValues )
{
    std::vector<Wt::WDateTime> wDateTimes;
    std::transform( dateTimes.begin(), dateTimes.end(), std::back_inserter( wDateTimes ), []( const QDateTime& dateTime ) {
        return Wt::WDateTime::fromTime_t( dateTime.toTime_t() );
    } );
    setSamples( wDateTimes, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                  const std::vector<double>& yValues,
                                                  bool                       keepOnlyPositiveValues )
{
    std::vector<Wt::WDateTime> wDateTimes;
    std::transform( dateTimes.begin(), dateTimes.end(), std::back_inserter( wDateTimes ), []( time_t time ) {
        return Wt::WDateTime::fromTime_t( time );
    } );
    setSamples( wDateTimes, yValues, keepOnlyPositiveValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename XDataType>
void WebPlotCurve::setSamples( const std::vector<XDataType>& xValues,
                               const std::vector<double>&    yValues,
                               bool                          keepOnlyPositiveValues )
{
    size_t valueCount = std::min( xValues.size(), yValues.size() );
    m_dataModel       = std::make_shared<Wt::WStandardItemModel>( (int)valueCount, 2 );
    for ( size_t i = 0u; i < xValues.size(); ++i )
    {
        m_dataModel->setData( (int)i, 0, xValues[i] );
        if ( CurveDataTools::isValidValue( yValues[i], keepOnlyPositiveValues ) )
        {
            m_dataModel->setData( (int)i, 1, yValues[i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::applyLineStyle( Wt::Chart::WDataSeries* dataSeries ) const
{
    CAF_ASSERT( dataSeries );
    Wt::WPen pen = dataSeries->pen();
    switch ( m_lineStyle )
    {
        case LineStyle::STYLE_NONE:
            pen.setStyle( Wt::PenStyle::None );
            break;
        case LineStyle::STYLE_SOLID:
            pen.setStyle( Wt::PenStyle::SolidLine );
            break;
        case LineStyle::STYLE_DASH:
            pen.setStyle( Wt::PenStyle::DashLine );
            break;
        case LineStyle::STYLE_DOT:
            pen.setStyle( Wt::PenStyle::DotLine );
            break;
        case LineStyle::STYLE_DASH_DOT:
            pen.setStyle( Wt::PenStyle::DashDotLine );
            break;
        default:
            break;
    }
    dataSeries->setPen( pen );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CurveInterface::LineStyle WebPlotCurve::getLineStyle( const Wt::Chart::WDataSeries* dataSeries )
{
    CAF_ASSERT( dataSeries );
    Wt::WPen pen = dataSeries->pen();

    switch ( pen.style() )
    {
        case Wt::PenStyle::None:
            return LineStyle::STYLE_NONE;
        case Wt::PenStyle::SolidLine:
            return LineStyle::STYLE_SOLID;
        case Wt::PenStyle::DashLine:
            return LineStyle::STYLE_DASH;
        case Wt::PenStyle::DotLine:
            return LineStyle::STYLE_DOT;
        case Wt::PenStyle::DashDotLine:
            return LineStyle::STYLE_DASH_DOT;
        case Wt::PenStyle::DashDotDotLine:
            break;
        default:
            break;
    }
    return LineStyle::STYLE_SOLID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WebPlotCurve::applyColor( Wt::Chart::WDataSeries* dataSeries ) const
{
    CAF_ASSERT( dataSeries );
    Wt::WPen pen = dataSeries->pen();
    pen.setColor( Wt::WColor( m_color.red(), m_color.green(), m_color.blue() ) );
    dataSeries->setPen( pen );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor WebPlotCurve::getColor( const Wt::Chart::WDataSeries* dataSeries )
{
    Wt::WColor penColor = dataSeries->pen().color();
    return QColor( penColor.red(), penColor.green(), penColor.blue() );
}
