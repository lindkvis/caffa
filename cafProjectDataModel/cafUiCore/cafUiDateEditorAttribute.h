#pragma once

#include "cafUiEditorAttribute.h"

#include <string>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class UiDateEditorAttribute : public UiEditorAttribute
{
public:
    std::string dateFormat;

public:
    UiDateEditorAttribute() {}
};

} // namespace caf
