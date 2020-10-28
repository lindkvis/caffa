//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafPdmUiOrdering.h"
#include "cafPdmWebWidgetObjectEditorHandle.h"

#include <QString>

#include <map>
#include <utility>

namespace Wt
{
class WContainerWidget;
class WGridLayout;
class WPanel;
class WContainerWidget;
class WWidget;
} // namespace Wt

namespace caf
{
class PdmWebFieldEditorHandle;
class PdmUiGroup;
class PdmUiOrdering;

//==================================================================================================
///
//==================================================================================================
class PdmWebFormLayoutObjectEditor : public PdmWebWidgetObjectEditorHandle
{
public:
    PdmWebFormLayoutObjectEditor();
    ~PdmWebFormLayoutObjectEditor() override;

protected:
    /// When overriding this function, use findOrCreateGroupBox() or findOrCreateFieldEditor() for detailed control
    /// Use recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn() for automatic layout of group and field widgets
    virtual void recursivelyConfigureAndUpdateTopLevelUiOrdering( const PdmUiOrdering& topLevelUiOrdering,
                                                                  const QString&       uiConfigName ) = 0;

    void                        recursivelyConfigureAndUpdateUiOrderingInGridLayout( const PdmUiOrdering& uiOrdering,
                                                                                     Wt::WGridLayout*     parentLayout,
                                                                                     const QString&       uiConfigName );
    std::unique_ptr<Wt::WPanel> recursivelyCreateGroup( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets,
                                                        PdmUiItem*                               currentItem,
                                                        const QString&                           uiConfigName );

    std::unique_ptr<Wt::WPanel> createGroupBox( PdmUiGroup* group, const QString& uiConfigName );
    PdmWebFieldEditorHandle*    findOrCreateFieldEditor( PdmFieldUiCapability* field, const QString& uiConfigName );

private:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    std::map<PdmFieldHandle*, PdmWebFieldEditorHandle*> m_fieldViews;
};

} // end namespace caf
