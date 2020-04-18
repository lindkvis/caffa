#pragma once

#include "cafPdmUiEditorAttribute.h"

#include <QString>

namespace caf {
//==================================================================================================
/// 
//==================================================================================================
class PdmUiDateEditorAttribute : public PdmUiEditorAttribute
{
public:
    QString dateFormat;

public:
    PdmUiDateEditorAttribute()
    {
    }
};

}