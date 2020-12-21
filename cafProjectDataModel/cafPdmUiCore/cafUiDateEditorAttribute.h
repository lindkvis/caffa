#pragma once

#include "cafUiEditorAttribute.h"

#include <string>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiDateEditorAttribute : public UiEditorAttribute
{
public:
    std::string dateFormat;

public:
    PdmUiDateEditorAttribute() {}
};

} // namespace caf
