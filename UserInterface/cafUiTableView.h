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
#include "cafUiFieldEditorHandle.h"

#include <QModelIndex>
#include <QString>
#include <QWidget>

class QTableView;

namespace caffa
{
class MenuInterface;
class ObjectHandle;
class UiTableViewEditor;
class ChildArrayFieldHandle;

//==================================================================================================
///
//==================================================================================================
class UiTableView : public QWidget
{
    Q_OBJECT
public:
    UiTableView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
    ~UiTableView() override;

    void setChildArrayField( ChildArrayFieldHandle* childArrayField );
    void enableHeaderText( bool enable );
    void setTableSelectionLevel( int selectionLevel );
    void setRowSelectionLevel( int selectionLevel );

    ObjectHandle* objectFromModelIndex( const QModelIndex& mi );

    QTableView* tableView();

    static void addActionsToMenu( MenuInterface* menu, ChildArrayFieldHandle* childArrayField );

private:
    UiTableViewEditor* m_listViewEditor;
    QString            m_;
};

} // End of namespace caffa
