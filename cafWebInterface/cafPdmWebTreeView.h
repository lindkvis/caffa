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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafPdmWebTreeViewEditor.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WPanel.h>
#include <Wt/WSignal.h>

#include <memory>

namespace Wt
{
class WTreeView;
}

namespace caf
{
class UiItem;
class PdmWebTreeViewEditor;
class PdmUiDragDropInterface;
class ObjectHandle;

//==================================================================================================
///
//==================================================================================================

class PdmWebTreeView : public Wt::WPanel
{
public:
    PdmWebTreeView();
    ~PdmWebTreeView() override;

    void enableDefaultContextMenu( bool enable );
    void enableSelectionManagerUpdating( bool enable ); // TODO: rename
    void enableAppendOfClassNameToUiItemText( bool enable );

    void setPdmItem( caf::UiItem* object );

    Wt::WTreeView* treeView();

    void selectedUiItems( std::vector<UiItem*>& objects ); // TODO: rename
    void selectAsCurrentItem( const UiItem* uiItem );
    void selectItems( const std::vector<const UiItem*>& uiItems );
    void setExpanded( const UiItem* uiItem, bool doExpand ) const;

    // ModelIndex access
    // Use this translation only when it is inconvenient to traverse
    // the Pdm model directly.
    UiItem*         uiItemFromModelIndex( const Wt::WModelIndex& index ) const;
    Wt::WModelIndex findModelIndex( const UiItem* object ) const;

    // void        setDragDropInterface(PdmUiDragDropInterface* dragDropInterface);

    // signals:
    Wt::Signal<>& selectionChanged();
    // Convenience signal for use with PdmUiPropertyView
    Wt::Signal<caf::ObjectHandle*>& selectedObjectChanged();

private: // slots:
    void slotOnSelectionChanged();

private:
    std::unique_ptr<PdmWebTreeViewEditor> m_treeViewEditor;

    Wt::Signal<>                   m_selectionChanged;
    Wt::Signal<caf::ObjectHandle*> m_selectedObjectChanged;
};

} // End of namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
