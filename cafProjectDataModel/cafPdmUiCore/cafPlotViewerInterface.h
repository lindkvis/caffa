#pragma once

#include "cafCurveInterface.h"

#include <QColor>
#include <QString>

#include <list>
#include <utility>

namespace caf {

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
    virtual void    setTitle(const QString& plotTitle) = 0;
    virtual QString title() const = 0;

    virtual void setOrientation(Orientation orientation) = 0;
    virtual Orientation orientation() const = 0;

    virtual void   setBackgroundColor(const QColor& color) = 0;
    virtual QColor backgroundColor() const = 0;

    virtual std::pair<double, double> axisRange(Axis axis) const = 0;
    virtual void setAxisRange(Axis axis, double minValue, double maxValue) = 0;

    virtual void setAxisTitle(Axis axis, const QString& axisTitle) = 0;
    virtual QString axisTitle(Axis axis) const = 0;

    virtual void setLegendEnabled(bool enabled) = 0;
    virtual bool legendEnabled() const = 0;

    virtual void setZoomEnabled(bool enabled) = 0;
    virtual bool zoomEnabled() const = 0;
    virtual void setPanEnabled(bool enabled) = 0;
    virtual bool panEnabled() const = 0;

    virtual void addCurve(std::shared_ptr<CurveInterface> curveToAdd) = 0;
    virtual void removeCurve(CurveInterface* curveToRemove) = 0;
    virtual void removeAllCurves() = 0;

    virtual std::list<std::shared_ptr<CurveInterface>> curves() const = 0;

    virtual void updatePlot() = 0;

};
}

