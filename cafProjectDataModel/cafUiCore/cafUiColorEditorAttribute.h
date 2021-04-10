#pragma once

#include "cafUiEditorAttribute.h"

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class UiColorEditorAttribute : public UiEditorAttribute
{
public:
    bool showAlpha;
    bool showLabel;

public:
    UiColorEditorAttribute()
    {
        showAlpha = false;
        showLabel = true;
    }
};
} // namespace caf