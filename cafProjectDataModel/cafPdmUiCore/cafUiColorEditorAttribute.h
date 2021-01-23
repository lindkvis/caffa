#pragma once

#include "cafUiEditorAttribute.h"

namespace caf {
//==================================================================================================
/// 
//==================================================================================================
class PdmUiColorEditorAttribute : public UiEditorAttribute
{
public:
    bool showAlpha;
    bool showLabel;

public:
    PdmUiColorEditorAttribute()
    {
        showAlpha = false;
        showLabel = true;
    }
};
}