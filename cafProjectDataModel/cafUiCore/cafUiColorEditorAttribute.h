#pragma once

#include "cafUiEditorAttribute.h"

namespace caffa
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
} // namespace caffa