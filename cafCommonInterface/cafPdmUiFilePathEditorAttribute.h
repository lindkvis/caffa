#pragma once

#include "cafPdmUiEditorAttribute.h"

#include <QString>

namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiFilePathEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiFilePathEditorAttribute()
    {
        m_selectSaveFileName = false;
        m_fileSelectionFilter = "All files (*.*)";
        m_defaultPath = QString();
        m_selectDirectory = false;
        m_appendUiSelectedFolderToText = false;
        m_multipleItemSeparator = ';';
    }
public:
    bool    m_selectSaveFileName;
    QString m_fileSelectionFilter;

    QString m_defaultPath;
    bool    m_selectDirectory;
    bool    m_appendUiSelectedFolderToText;
    QChar   m_multipleItemSeparator;
};

}

