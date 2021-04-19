#pragma once

#include "cafUiEditorAttribute.h"

#include <string>

namespace caffa
{
//==================================================================================================
///
//==================================================================================================
class UiFilePathEditorAttribute : public UiEditorAttribute
{
public:
    UiFilePathEditorAttribute()
    {
        m_selectSaveFileName           = false;
        m_fileSelectionFilter          = "All files (*.*)";
        m_defaultPath                  = std::string();
        m_selectDirectory              = false;
        m_appendUiSelectedFolderToText = false;
        m_multipleItemSeparator        = ';';
    }

public:
    bool        m_selectSaveFileName;
    std::string m_fileSelectionFilter;

    std::string m_defaultPath;
    bool        m_selectDirectory;
    bool        m_appendUiSelectedFolderToText;
    char        m_multipleItemSeparator;
};

} // namespace caffa
