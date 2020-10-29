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
#include "cafUserInterface_export.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiTreeEditorHandle.h"
#include "cafUiTreeViewQModel.h"

#include "cafUiTreeViewAttribute.h"
#include <QAbstractItemModel>
#include <QColor>
#include <QItemSelectionModel>
#include <QPointer>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>

class MySortFilterProxyModel;

class QGridLayout;
class QTreeView;
class QVBoxLayout;

namespace caf
{
class ChildArrayFieldHandle;
class PdmUiDragDropInterface;
class UiItem;
class PdmUiTreeViewEditor;
class PdmUiTreeViewQModel;
class PdmUiTreeViewWidget;

class PdmUiTreeViewItemDelegate : public QStyledItemDelegate
{
public:
    PdmUiTreeViewItemDelegate( QObject* parent, PdmUiTreeViewQModel* model );
    void clearAttributes();
    void addAttribute( QModelIndex index, const PdmUiTreeViewItemAttribute& attribute );
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;

private:
    PdmUiTreeViewQModel*                              m_model;
    std::map<QModelIndex, PdmUiTreeViewItemAttribute> m_attributes;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class cafUserInterface_EXPORT PdmUiTreeViewEditor : public PdmUiTreeEditorHandle
{
    Q_OBJECT
public:
    PdmUiTreeViewEditor();
    ~PdmUiTreeViewEditor() override;

    void enableDefaultContextMenu( bool enable );
    void enableSelectionManagerUpdating( bool enable );

    void enableAppendOfClassNameToUiItemText( bool enable );
    bool isAppendOfClassNameToUiItemTextEnabled();

    QTreeView* treeView();
    bool       isTreeItemEditWidgetActive() const;

    void selectAsCurrentItem( const UiItem* uiItem );
    void selectItems( std::vector<const UiItem*> uiItems );
    void selectedUiItems( std::vector<UiItem*>& objects );
    void setExpanded( const UiItem* uiItem, bool doExpand ) const;

    UiItem*  uiItemFromModelIndex( const QModelIndex& index ) const;
    QModelIndex findModelIndex( const UiItem* object ) const;

    QWidget* createWidget( QWidget* parent ) override;

    void setDragDropInterface( PdmUiDragDropInterface* dragDropInterface );

signals:
    void selectionChanged();

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

    void updateMySubTree( UiItem* uiItem ) override;

    void updateContextMenuSignals();

private slots:
    void customMenuRequested( QPoint pos );
    void slotOnSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private:
    ChildArrayFieldHandle* currentChildArrayFieldHandle();

    void updateSelectionManager();
    void updateItemDelegateForSubTree( const QModelIndex& modelIndex = QModelIndex() );

    bool eventFilter( QObject* obj, QEvent* event ) override;

private:
    QPointer<QWidget> m_mainWidget;
    QVBoxLayout*      m_layout;

    PdmUiTreeViewWidget*       m_treeView;
    PdmUiTreeViewQModel*       m_treeViewModel;
    PdmUiTreeViewItemDelegate* m_delegate;

    bool m_useDefaultContextMenu;
    bool m_updateSelectionManager;
    bool m_appendClassNameToUiItemText;
};

} // end namespace caf