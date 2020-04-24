#pragma once

#include "cafCurveInterface.h"

#include <QColor>
#include <QString>

#include <list>
#include <map>
#include <memory>
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

public:
    PlotViewerInterface();
    virtual ~PlotViewerInterface();

    virtual void    setTitle( const QString& plotTitle );
    virtual QString title() const;

    virtual void        setOrientation( Orientation orientation );
    virtual Orientation orientation() const;

    virtual void   setBackgroundColor( const QColor& color );
    virtual QColor backgroundColor() const;

    virtual std::pair<double, double> axisRange( Axis axis ) const;
    virtual void                      setAxisRange( Axis axis, double minValue, double maxValue );

    virtual void    setAxisTitle( Axis axis, const QString& axisTitle );
    virtual QString axisTitle( Axis axis ) const;

    virtual void setLegendEnabled( bool enabled );
    virtual bool legendEnabled() const;

    virtual void setZoomEnabled( bool enabled );
    virtual bool zoomEnabled() const;
    virtual void setPanEnabled( bool enabled );
    virtual bool panEnabled() const;

    virtual void addCurve( std::shared_ptr<CurveInterface> curveToAdd );
    virtual void removeCurve( CurveInterface* curveToRemove );
    virtual void removeAllCurves();

    virtual std::list<std::shared_ptr<CurveInterface>> curves() const;

    virtual void updateViewer() = 0;

protected:
    std::list<std::shared_ptr<CurveInterface>> m_curves;

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
