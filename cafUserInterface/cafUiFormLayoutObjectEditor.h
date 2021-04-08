//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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
#include "cafFieldUiCapability.h"
#include "cafUiOrdering.h"
#include "cafUiWidgetObjectEditorHandle.h"

#include <QObject>
#include <QPointer>
#include <QString>

#include <map>

class QMinimizePanel;
class QGridLayout;
class QWidget;

namespace caf
{
class UiFieldEditorHandle;
class UiGroup;
class UiOrdering;

//==================================================================================================
///
//==================================================================================================
class PdmUiFormLayoutObjectEditor : public QObject, public PdmUiWidgetObjectEditorHandle
{
    Q_OBJECT

public:
    PdmUiFormLayoutObjectEditor();
    ~PdmUiFormLayoutObjectEditor() override;

protected:
    /// When overriding this function, use findOrCreateGroupBox() or findOrCreateFieldEditor() for detailed control
    /// Use recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn() for automatic layout of group and field widgets
    virtual void recursivelyConfigureAndUpdateTopLevelUiOrdering( const UiOrdering& topLevelUiOrdering ) = 0;

    bool recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( const UiOrdering& uiOrdering, QWidget* containerWidget );
    int recursivelyConfigureAndUpdateUiOrderingInGridLayout( const UiOrdering& uiOrdering,
                                                             QWidget*          containerWidgetWithGridLayout );

    int recursivelyAddGroupToGridLayout( UiItem*      currentItem,
                                         QWidget*     containerWidget,
                                         QGridLayout* parentLayout,
                                         int          currentRowIndex,
                                         int          currentColumn,
                                         int          itemColumnSpan );

    QMinimizePanel*      findOrCreateGroupBox( QWidget* parent, UiGroup* group );
    UiFieldEditorHandle* findOrCreateFieldEditor( QWidget* parent, FieldUiCapability* field );

    static void ensureWidgetContainsEmptyGridLayout( QWidget* containerWidget, QMargins contentMargins = QMargins() );

public slots:
    void groupBoxExpandedStateToggled( bool isExpanded );

private:
    bool isUiGroupExpanded( const UiGroup* uiGroup ) const;
    void cleanupBeforeSettingObject() override;
    void configureAndUpdateUi() override;

private:
    std::map<FieldHandle*, UiFieldEditorHandle*> m_fieldViews;
    std::set<FieldHandle*> m_usedFields; ///< used temporarily to store the new(complete) set of used fields
    std::map<std::string, QPointer<QMinimizePanel>> m_groupBoxes;
    std::map<std::string, QPointer<QMinimizePanel>> m_newGroupBoxes; ///< used temporarily to store the new(complete)
                                                                     ///< set of group boxes
    std::map<std::string, std::map<std::string, bool>> m_objectKeywordGroupUiNameExpandedState;
};

} // end namespace caf
