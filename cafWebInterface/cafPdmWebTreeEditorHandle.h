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

#include "cafPdmUiEditorHandle.h"
#include "cafPdmPointer.h"

#include <QString>

#include <Wt/Core/observing_ptr.hpp>
#include <Wt/WContainerWidget.h>

#include <vector>

namespace caf 
{

class ObjectHandle;

//==================================================================================================
/// 
//==================================================================================================

class PdmWebTreeEditorHandle: public PdmUiEditorHandle
{
public:
    PdmWebTreeEditorHandle() {}
    ~PdmWebTreeEditorHandle() override {}
   
    Wt::WWidget* getOrCreateWidget();
    Wt::WWidget* widget();

    void                setPdmItemRoot(PdmUiItem* root);
    PdmUiItem*          pdmItemRoot();
    void                updateSubTree(PdmUiItem* root) { this->updateMySubTree(root); }

protected:
    virtual Wt::WWidget* createWidget() = 0;

    /// Supposed to update the representation of the tree from root and downwards, as gracefully as possible.
    /// Will be called when the content of root might have been changed
    virtual void        updateMySubTree(PdmUiItem* root) = 0;

protected:
    Wt::Core::observing_ptr<Wt::WWidget> m_widget;
};



} // End of namespace caf

