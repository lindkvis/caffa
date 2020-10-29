#pragma once

#include "cafObjectHandle.h"
#include "cafUiEditorAttribute.h"

#include <QColor>
#include <QString>

namespace caf
{

class PdmUiTreeViewItemAttribute : public UiEditorAttribute
{
public:
    enum Position
    {
        IN_FRONT,
        AT_END
    };
    PdmUiTreeViewItemAttribute()
        : tag()
        , position(AT_END)
        , bgColor(Qt::red)
        , fgColor(Qt::white)
    {

    }
    QString  tag;
    Position position;
    QColor   bgColor;
    QColor   fgColor;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewEditorAttribute : public UiEditorAttribute
{
public:
    PdmUiTreeViewEditorAttribute()
    {
        currentObject = nullptr;
    }

public:
    QStringList columnHeaders;

    /// This object is set as current item in the tree view in configureAndUpdateUi()
    caf::ObjectHandle* currentObject;
};
}