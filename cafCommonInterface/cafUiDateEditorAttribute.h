#pragma once

#include "cafUiEditorAttribute.h"

#include <QString>

namespace caf {
//==================================================================================================
/// 
//==================================================================================================
class PdmUiDateEditorAttribute : public UiEditorAttribute
{
public:
    QString dateFormat;

public:
    PdmUiDateEditorAttribute()
    {
    }
};

}