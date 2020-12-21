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

#include "WPopupMenuWrapper.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafPdmWebTreeEditorHandle.h"
#include "cafPdmWebTreeViewWModel.h"

#include <Wt/WAbstractItemModel.h>
#include <Wt/WColor.h>
#include <Wt/WItemSelectionModel.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WTreeView.h>
#include <Wt/WWidget.h>

class MySortFilterProxyModel;

namespace caf
{
class ChildArrayFieldHandle;
class UiItem;
class PdmWebTreeViewEditor;
class PdmWebTreeViewWModel;
class PdmUiTreeViewWidget;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmWebTreeViewEditor : public PdmWebTreeEditorHandle
{
public:
    PdmWebTreeViewEditor();
    ~PdmWebTreeViewEditor() override;

    void enableDefaultContextMenu( bool enable );
    void enableSelectionManagerUpdating( bool enable );

    void enableAppendOfClassNameToUiItemText( bool enable );
    bool isAppendOfClassNameToUiItemTextEnabled();

    Wt::WTreeView* treeView();

    void selectAsCurrentItem( const UiItem* uiItem );
    void selectItems( std::vector<const UiItem*> uiItems );
    void selectedUiItems( std::vector<UiItem*>& objects );
    void setExpanded( const UiItem* uiItem, bool doExpand ) const;

    UiItem*         uiItemFromModelIndex( const Wt::WModelIndex& index ) const;
    Wt::WModelIndex findModelIndex( const UiItem* object ) const;

    Wt::WWidget* createWidget() override;

    // void setDragDropInterface( PdmUiDragDropInterface* dragDropInterface );

    Wt::Signal<>& selectionChanged();

protected:
    void configureAndUpdateUi() override;
    void updateMySubTree( UiItem* uiItem ) override;
    void updateContextMenuSignals();

    void slotCustomMenuRequested( const Wt::WModelIndex& item, const Wt::WMouseEvent& event );
    void slotOnSelectionChanged();
    void slotOnActionSelection();

private:
    ChildArrayFieldHandle* currentChildArrayFieldHandle();

    void updateSelectionManager();

private:
    Wt::Core::observing_ptr<Wt::WTreeView> m_treeView;
    std::shared_ptr<PdmWebTreeViewWModel>  m_treeViewModel;
    Wt::Signal<>                           m_selectionChanged;
    std::unique_ptr<WPopupMenuWrapper>     m_popup;

    bool m_useDefaultContextMenu;
    bool m_updateSelectionManager;
    bool m_appendClassNameToUiItemText;
};

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
