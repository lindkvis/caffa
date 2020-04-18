#pragma once

#include "cafPdmUiEditorAttribute.h"

namespace caf {
//==================================================================================================
/// 
//==================================================================================================
class PdmUiColorEditorAttribute : public PdmUiEditorAttribute
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