#pragma once

#include "cafObjectHandle.h"
#include "cafUiEditorAttribute.h"

#include <string>
#include <vector>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiTreeViewEditorAttribute : public UiEditorAttribute
{
public:
    UiTreeViewEditorAttribute() { currentObject = nullptr; }

public:
    std::vector<std::string> columnHeaders;

    /// This object is set as current item in the tree view in configureAndUpdateUi()
    caffa::ObjectHandle* currentObject;
};
} // namespace caffa