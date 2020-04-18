#pragma once

#include "cafAssert.h"

#include <QColor>
#include <QDateTime>

#include <iostream>
#include <utility>
#include <vector>

namespace caf {

class PlotViewerInterface;

class CurveInterface
{
public:
     enum class LineStyle
     {
       STYLE_NONE,
       STYLE_SOLID,
       STYLE_DASH,
       STYLE_DOT,
       STYLE_DASH_DOT
     };

public:
    virtual void attachToPlot(PlotViewerInterface* plot) = 0;
    virtual void detachFromPlot() = 0;

    virtual void setName(const QString& title) = 0;
    virtual QString name() const = 0;

    virtual void setLineStyle(LineStyle lineStyle) = 0;
    virtual LineStyle lineStyle() const = 0;
    virtual void setColor(const QColor& color) = 0;
    virtual QColor color() const = 0;

    virtual void setSamplesFromXValuesAndYValues(
        const std::vector<double>& xValues,
        const std::vector<double>& yValues,
        bool keepOnlyPositiveValues = false) = 0;

    virtual void setSamplesFromDatesAndYValues(
        const std::vector<QDateTime>& dateTimes,
        const std::vector<double>& yValues,
        bool keepOnlyPositiveValues = false) = 0;

    virtual void setSamplesFromTimeTAndYValues(
        const std::vector<time_t>& dateTimes,
        const std::vector<double>& yValues,
        bool keepOnlyPositiveValues = false) = 0;

    virtual size_t sampleCount() const = 0;

protected:
    std::vector<std::pair<size_t, size_t>> m_polyLineStartStopIndices;
};
}
