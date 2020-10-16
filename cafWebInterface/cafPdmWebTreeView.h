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

#include "cafPdmWebTreeViewEditor.h"

#include <QString>
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

class PdmUiItem;
class PdmWebTreeViewEditor;
class PdmUiDragDropInterface;
class PdmObjectHandle;

//==================================================================================================
/// 
//==================================================================================================

class PdmWebTreeView : public Wt::WPanel
{
public:
    PdmWebTreeView();
    ~PdmWebTreeView() override;

    void        enableDefaultContextMenu(bool enable);
    void        enableSelectionManagerUpdating(bool enable); // TODO: rename
    void        enableAppendOfClassNameToUiItemText(bool enable);

    void        setUiConfigurationName(QString uiConfigName);
    void        setPdmItem(caf::PdmUiItem* object);

    Wt::WTreeView*  treeView();

    void        selectedUiItems(std::vector<PdmUiItem*>& objects); // TODO: rename
    void        selectAsCurrentItem(const PdmUiItem* uiItem);
    void        selectItems(const std::vector<const PdmUiItem*>& uiItems);
    void        setExpanded(const PdmUiItem* uiItem, bool doExpand) const ;

    // QModelIndex access
    // Use this translation only when it is inconvenient to traverse 
    // the Pdm model directly.
    PdmUiItem*  uiItemFromModelIndex(const Wt::WModelIndex& index) const;
    Wt::WModelIndex findModelIndex(const PdmUiItem* object) const;

    //void        setDragDropInterface(PdmUiDragDropInterface* dragDropInterface);

//signals:
    Wt::Signal<>& selectionChanged();
    // Convenience signal for use with PdmUiPropertyView
    Wt::Signal<caf::PdmObjectHandle*>& selectedObjectChanged();

private: // slots:
    void        slotOnSelectionChanged();

private:
    std::unique_ptr<PdmWebTreeViewEditor>   m_treeViewEditor; 
    QString                                 m_uiConfigName;

    Wt::Signal<>                      m_selectionChanged;
    Wt::Signal<caf::PdmObjectHandle*> m_selectedObjectChanged;
};



} // End of namespace caf

