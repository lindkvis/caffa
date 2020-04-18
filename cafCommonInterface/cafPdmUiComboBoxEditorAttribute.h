//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cafPdmUiEditorAttribute.h"

#include <QSize>
#include <QString>

namespace caf
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiComboBoxEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiComboBoxEditorAttribute()
    {
        adjustWidthToContents = false;
        showPreviousAndNextButtons = false;
        minimumContentsLength = 8;
        maximumMenuContentsLength = 40;
        enableEditableContent = false;
        minimumWidth = -1;
        iconSize = QSize(14, 14);
    }

public:
    bool    adjustWidthToContents;
    bool    showPreviousAndNextButtons;
    int     minimumContentsLength; // The length of string to adjust to if adjustWidthToContents = false.
                                    // Set to <= 0 to ignore and use AdjustToContentsOnFirstShow instead
    int     maximumMenuContentsLength;
    bool    enableEditableContent;
    int     minimumWidth;
    QString placeholderText;
    QString nextButtonText;
    QString prevButtonText;

    QSize   iconSize;
};
}

