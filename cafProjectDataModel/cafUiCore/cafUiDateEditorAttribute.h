#pragma once

#include "cafUiEditorAttribute.h"

#include <string>

namespace caffa
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

} // namespace caffa
