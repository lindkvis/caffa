//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafPdmDocument.h"
#include "cafSelectionChangedReceiver.h"
#include "cafUiFieldEditorHandle.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QItemSelection;
class QLabel;
class QTableView;

namespace caf
{
class PdmUiCheckBoxDelegate;
class UiFieldEditorHandle;
class UiItem;
class PdmUiTableViewDelegate;
class PdmUiTableViewQModel;
class ChildArrayFieldHandle;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewPushButtonEditorAttribute : public UiEditorAttribute
{
public:
    void registerPushButtonTextForFieldKeyword( const std::string& keyword, const std::string& text );

    bool        showPushButtonForFieldKeyword( const std::string& keyword ) const;
    std::string pushButtonText( const std::string& keyword ) const;

private:
    std::map<std::string, std::string> m_fieldKeywordAndPushButtonText;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewEditorAttribute : public UiEditorAttribute
{
public:
    enum ResizePolicy
    {
        NO_AUTOMATIC_RESIZE,
        RESIZE_TO_FIT_CONTENT,
        RESIZE_TO_FILL_CONTAINER
    };

    PdmUiTableViewEditorAttribute()
        : tableSelectionLevel( 0 )
        , rowSelectionLevel( 1 )
        , enableHeaderText( true )
        , minimumHeight( -1 )
        , alwaysEnforceResizePolicy( false )
        , resizePolicy( NO_AUTOMATIC_RESIZE )
    {
        QPalette myPalette;
        baseColor = myPalette.color( QPalette::Active, QPalette::Base );
    }

    int              selectionLevel;
    int              tableSelectionLevel;
    int              rowSelectionLevel;
    bool             enableHeaderText;
    std::vector<int> columnWidths;
    int              minimumHeight; ///< Not used if If < 0
    QColor           baseColor;
    bool             alwaysEnforceResizePolicy;
    ResizePolicy     resizePolicy;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

class PdmUiTableViewEditor : public UiFieldEditorHandle, public SelectionChangedReceiver
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTableViewEditor();
    ~PdmUiTableViewEditor() override;

    void enableHeaderText( bool enable );
    void setTableSelectionLevel( int selectionLevel );
    void setRowSelectionLevel( int selectionLevel );

    ObjectHandle* pdmObjectFromModelIndex( const QModelIndex& mi );
    QTableView*   tableView();

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;

    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;
    bool isMultiRowEditor() const override;

private:
    void selectedUiItems( const QModelIndexList& modelIndexList, std::vector<UiItem*>& objects );
    bool isSelectionRoleDefined() const;
    void updateSelectionManagerFromTableSelection();

    bool                   eventFilter( QObject* obj, QEvent* event ) override;
    ChildArrayFieldHandle* childArrayFieldHandle();

private slots:
    void slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private:
    friend class FocusEventHandler;

    QPointer<QLabel> m_tableHeading;
    QPointer<QLabel> m_tableHeadingIcon;

    QTableView*           m_tableView;
    PdmUiTableViewQModel* m_tableModelPdm;

    PdmUiTableViewDelegate* m_delegate;
    PdmUiCheckBoxDelegate*  m_checkboxDelegate;

    bool m_useDefaultContextMenu;
    int  m_tableSelectionLevel;
    int  m_rowSelectionLevel;
    bool m_isBlockingSelectionManagerChanged;
    bool m_isUpdatingSelectionQModel;

    caf::ChildArrayFieldHandle* m_previousFieldHandle;
};

} // end namespace caf
