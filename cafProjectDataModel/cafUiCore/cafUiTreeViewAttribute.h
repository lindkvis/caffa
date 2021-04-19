#pragma once

#include "cafColor.h"
#include "cafObjectHandle.h"
#include "cafUiEditorAttribute.h"

#include <string>
#include <vector>

namespace caffa
{
class UiTreeViewItemAttribute : public UiEditorAttribute
{
public:
    enum Position
    {
        IN_FRONT,
        AT_END
    };
    UiTreeViewItemAttribute()
        : tag()
        , position( AT_END )
        , bgColor( 255, 0, 0 )
        , fgColor( 255, 255, 255 )
    {
    }
    std::string tag;
    Position    position;
    Color       bgColor;
    Color       fgColor;
};

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