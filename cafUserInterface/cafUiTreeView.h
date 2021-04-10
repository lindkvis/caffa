//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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
#include <QString>
#include <QWidget>

class QVBoxLayout;
class QTreeView;
class QItemSelection;
class QMenu;
class QModelIndex;

namespace caf
{
class UiItem;
class UiTreeViewEditor;
class UiDragDropInterface;
class ObjectHandle;

//==================================================================================================
///
//==================================================================================================

class UiTreeView : public QWidget
{
    Q_OBJECT
public:
    UiTreeView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
    ~UiTreeView() override;

    void enableDefaultContextMenu( bool enable );
    void enableSelectionManagerUpdating( bool enable ); // TODO: rename
    void enableAppendOfClassNameToUiItemText( bool enable );

    void setItem( caf::UiItem* object );

    QTreeView* treeView();
    bool       isTreeItemEditWidgetActive() const;

    void selectedUiItems( std::vector<UiItem*>& objects ); // TODO: rename
    void selectAsCurrentItem( const UiItem* uiItem );
    void selectItems( const std::vector<const UiItem*>& uiItems );
    void setExpanded( const UiItem* uiItem, bool doExpand ) const;

    // QModelIndex access
    // Use this translation only when it is inconvenient to traverse
    // the Pdm model directly.
    UiItem*     uiItemFromModelIndex( const QModelIndex& index ) const;
    QModelIndex findModelIndex( const UiItem* object ) const;

    void setDragDropInterface( UiDragDropInterface* dragDropInterface );

signals:
    void selectionChanged();
    // Convenience signal for use with UiPropertyView
    void selectedObjectChanged( caf::ObjectHandle* object ); // Signal/Slot system needs caf:: prefix in some cases

private slots:
    void slotOnSelectionChanged();

private:
    UiTreeViewEditor* m_treeViewEditor;
    QString           m_;
    QVBoxLayout*      m_layout;
};

} // End of namespace caf
