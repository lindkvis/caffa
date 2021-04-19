#pragma once

#include "cafUiEditorAttribute.h"

namespace caffa
{
//==================================================================================================
///
//==================================================================================================
class UiSliderEditorAttribute : public UiEditorAttribute
{
public:
    UiSliderEditorAttribute()
    {
        m_minimum                       = 0;
        m_maximum                       = 10;
        m_delaySliderUpdateUntilRelease = false;
    }

public:
    int  m_minimum;
    int  m_maximum;
    bool m_delaySliderUpdateUntilRelease;
};

//==================================================================================================
///
//==================================================================================================
class UiDoubleSliderEditorAttribute : public UiEditorAttribute
{
public:
    UiDoubleSliderEditorAttribute()
    {
        m_minimum                       = 0;
        m_maximum                       = 10;
        m_decimals                      = 6;
        m_sliderTickCount               = 2000;
        m_delaySliderUpdateUntilRelease = false;
    }

public:
    double m_minimum;
    double m_maximum;
    int    m_decimals;
    int    m_sliderTickCount;
    bool   m_delaySliderUpdateUntilRelease;
};
} // namespace caffa
