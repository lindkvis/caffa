#pragma once

#include "cafCurveInterface.h"
#include "cafFontTools.h"

#include <QColor>
#include <QString>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>

namespace caf
{
class CurveInterface;

class PlotViewerInterface
{
public:
    enum class ChartType
    {
        Category,
        Scatter
    };

    enum class Axis
    {
        yLeft,
        yRight,
        x
    };

    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    struct AxisProperties
    {
        AxisProperties();
        QString title;
        bool    titleEnabled;
        bool    titleBold;
        int     titleAlignment;

        FontTools::Size titleFontSize;
        FontTools::Size valueFontSize;

        bool   inverted;
        bool   labelsEnabled;
        bool   ticksEnabled;
        double labelAngleDegrees;

        bool majorGridLines;
        bool minorGridLines;

        bool                      autoScale;
        std::pair<double, double> range;
    };

public:
    PlotViewerInterface();
    virtual ~PlotViewerInterface();

    void    setTitle( const QString& plotTitle );
    QString title() const;
    void    setTitleEnabled( bool enabled );
    bool    titleEnabled() const;

    void        setOrientation( Orientation orientation );
    Orientation orientation() const;

    void   setBackgroundColor( const QColor& color );
    QColor backgroundColor() const;

    std::pair<double, double> axisRange( Axis axis ) const;
    void                      setAxisRange( Axis axis, double minValue, double maxValue );

    void    setAxisTitle( Axis axis, const QString& axisTitle );
    QString axisTitle( Axis axis ) const;
    void    setAxisTitleEnabled( Axis axis, bool enabled );
    bool    axisTitleEnabled( Axis axis ) const;
    void    setAxisInverted( Axis axis, bool inverted = true );
    void    setAxisLabelsAndTicksEnabled( Axis axis, bool enableLabels, bool enableTicks );

    FontTools::Size axisTitleFontPointSize( Axis axis ) const;
    FontTools::Size axisValueFontPointSize( Axis axis ) const;

    void setAxisFontsAndAlignment( Axis            axis,
                                   FontTools::Size titleFontSize,
                                   FontTools::Size valueFontPointSize,
                                   bool            titleBold = false,
                                   int             alignment = (int)Qt::AlignRight );

    void enableGridLines( Axis axis, bool majorGridLines, bool minorGridLines );

    void setLegendEnabled( bool enabled );
    bool legendEnabled() const;
    void setLegendFontPointSize( int fontSizePt );

    void setZoomEnabled( bool enabled );
    bool zoomEnabled() const;

    void setPanEnabled( bool enabled );
    bool panEnabled() const;

    void addCurve( std::shared_ptr<CurveInterface> curveToAdd );
    void removeCurve( CurveInterface* curveToRemove );
    void removeAllCurves();

    std::list<std::shared_ptr<CurveInterface>> curves() const;

protected:
    AxisProperties& axisProperties( Axis axis );
    AxisProperties  axisProperties( Axis axis ) const;

private:
    virtual void dataChanged( void* changedVariable = nullptr )                            = 0;
    virtual void assignPropertiesToAxis( Axis axisEnum, const AxisProperties& properties ) = 0;

protected:
    std::list<std::shared_ptr<CurveInterface>> m_curves;

    QString m_title;
    bool    m_titleEnabled;

    Orientation m_orientation;
    QColor      m_bgColor;

    std::map<Axis, AxisProperties> m_axisProperties;

    int m_legendFontSizePt;

    bool m_legendEnabled;
    bool m_zoomEnabled;
    bool m_panEnabled;
};
} // namespace caf
