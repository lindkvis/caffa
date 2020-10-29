//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) Ceetron AS
//   Copyright (C) Gaute Lindkvist
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

#include "cafUiFilePathEditorAttribute.h"
#include "cafPdmWebFieldEditorHandle.h"

#include <Wt/Core/observing_ptr.hpp>
#include <Wt/WAnchor.h>
#include <Wt/WFileUpload.h>
#include <Wt/WLabel.h>
#include <Wt/WPushButton.h>

#include <QDir>

namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmWebFileUploadEditor : public PdmWebFieldEditorHandle
{
    CAF_PDM_WEB_FIELD_EDITOR_HEADER_INIT;

public:
    PdmWebFileUploadEditor()          {} 
    ~PdmWebFileUploadEditor() override {} 

protected:
    Wt::WWidget*    createEditorWidget() override;
    Wt::WLabel* createLabelWidget() override;
    void            configureAndUpdateUi(const QString& uiConfigName) override;

    void        slotFileSelectionOpen();
    void        slotFileSelectionFinished();
    void        slotFileUploaded();
    void        slotFileTooLargeError();

    static QDir uploadDir();
private:
    Wt::Core::observing_ptr<Wt::WLabel>      m_label;
    Wt::Core::observing_ptr<Wt::WAnchor>     m_fileLink;
    Wt::Core::observing_ptr<Wt::WFileUpload> m_fileUpload;
    Wt::Core::observing_ptr<Wt::WPushButton> m_uploadButton;

    PdmUiFilePathEditorAttribute m_attributes;
};


} // end namespace caf
