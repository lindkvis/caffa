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

#include "cafUiFormLayoutObjectEditor.h"

#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"
#include "cafObjectUiCapability.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiFieldEditorHelper.h"
#include "cafUiListEditor.h"
#include "cafUiOrdering.h"

#include "cafAssert.h"

#include "QMinimizePanel.h"

#include <QCoreApplication>
#include <QFrame>
#include <QGridLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiFormLayoutObjectEditor::UiFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiFormLayoutObjectEditor::~UiFormLayoutObjectEditor()
{
    // If there are field editor present, the usage of this editor has not cleared correctly
    // The intended usage is to call the method setObject(NULL) before closing the dialog
    CAF_ASSERT( m_fieldViews.size() == 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::UiFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( const UiOrdering& uiOrdering,
                                                                                            QWidget* containerWidget )
{
    QSize beforeSize = containerWidget->sizeHint();

    ensureWidgetContainsEmptyGridLayout( containerWidget );
    int stretch = recursivelyConfigureAndUpdateUiOrderingInGridLayout( uiOrdering, containerWidget );

    QSize afterSize = containerWidget->sizeHint();
    if ( beforeSize != afterSize )
    {
        containerWidget->adjustSize();
    }

    return stretch > 0;
}

//--------------------------------------------------------------------------------------------------
/// Add all widgets at a recursion level in the form.
/// Returns the stretch factor that should be applied at the level above.
//--------------------------------------------------------------------------------------------------
int caf::UiFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInGridLayout( const UiOrdering& uiOrdering,
                                                                                        QWidget* containerWidgetWithGridLayout )
{
    int sumRowStretch = 0;
    CAF_ASSERT( containerWidgetWithGridLayout );

    QWidget* previousTabOrderWidget = nullptr;

    // Currently, only QGridLayout is supported
    QGridLayout* parentLayout = dynamic_cast<QGridLayout*>( containerWidgetWithGridLayout->layout() );
    CAF_ASSERT( parentLayout );

    UiOrdering::TableLayout tableLayout = uiOrdering.calculateTableLayout();

    int totalRows    = static_cast<int>( tableLayout.size() );
    int totalColumns = uiOrdering.nrOfColumns( tableLayout );

    for ( int currentRowIndex = 0; currentRowIndex < totalRows; ++currentRowIndex )
    {
        int currentColumn = 0;

        const UiOrdering::RowLayout& uiItemsInRow = tableLayout[currentRowIndex];

        int columnsRequiredForCurrentRow = uiOrdering.nrOfRequiredColumnsInRow( uiItemsInRow );
        int nrOfExpandingItemsInRow      = uiOrdering.nrOfExpandingItemsInRow( uiItemsInRow );
        int spareColumnsInRow            = totalColumns - columnsRequiredForCurrentRow;

        std::div_t columnsDiv = { 0, 0 };
        if ( spareColumnsInRow && nrOfExpandingItemsInRow )
        {
            columnsDiv = std::div( spareColumnsInRow, nrOfExpandingItemsInRow );
        }

        for ( size_t i = 0; i < uiItemsInRow.size(); ++i )
        {
            UiItem*                   currentItem   = uiItemsInRow[i].first;
            UiOrdering::LayoutOptions currentLayout = uiItemsInRow[i].second;

            int minimumItemColumnSpan = 0, minimumLabelColumnSpan = 0, minimumFieldColumnSpan = 0;
            uiOrdering.nrOfColumnsRequiredForItem( uiItemsInRow[i],
                                                   &minimumItemColumnSpan,
                                                   &minimumLabelColumnSpan,
                                                   &minimumFieldColumnSpan );
            bool isExpandingItem = currentLayout.totalColumnSpan == UiOrdering::LayoutOptions::MAX_COLUMN_SPAN;

            int spareColumnsToAssign = 0;
            if ( isExpandingItem )
            {
                spareColumnsToAssign += columnsDiv.quot;
                if ( i == 0 ) spareColumnsToAssign += columnsDiv.rem;
            }

            int itemColumnSpan = minimumItemColumnSpan + spareColumnsToAssign;

            if ( currentItem->isUiGroup() )
            {
                int groupStretchFactor = recursivelyAddGroupToGridLayout( currentItem,
                                                                          containerWidgetWithGridLayout,
                                                                          parentLayout,
                                                                          currentRowIndex,
                                                                          currentColumn,
                                                                          itemColumnSpan );
                parentLayout->setRowStretch( currentRowIndex, groupStretchFactor );
                currentColumn += itemColumnSpan;
                sumRowStretch += groupStretchFactor;
            }
            else
            {
                UiFieldEditorHandle* fieldEditor = nullptr;
                FieldUiCapability*   field       = dynamic_cast<FieldUiCapability*>( currentItem );

                if ( field ) fieldEditor = findOrCreateFieldEditor( containerWidgetWithGridLayout, field );

                if ( fieldEditor )
                {
                    // Also assign required item space that isn't taken up by field and label
                    spareColumnsToAssign += minimumItemColumnSpan - ( minimumLabelColumnSpan + minimumFieldColumnSpan );

                    // Place the widget(s) into the correct parent and layout
                    QWidget* fieldCombinedWidget = fieldEditor->combinedWidget();

                    if ( fieldCombinedWidget )
                    {
                        parentLayout->addWidget( fieldCombinedWidget, currentRowIndex, currentColumn, 1, itemColumnSpan );
                        parentLayout->setRowStretch( currentRowIndex, fieldEditor->rowStretchFactor() );
                        sumRowStretch += fieldEditor->rowStretchFactor();
                    }
                    else
                    {
                        QWidget* fieldEditorWidget = fieldEditor->editorWidget();
                        if ( !fieldEditorWidget ) continue;

                        int fieldColumnSpan = minimumFieldColumnSpan;

                        QWidget*                 fieldLabelWidget = fieldEditor->labelWidget();
                        UiItemInfo::LabelPosType labelPos         = UiItemInfo::HIDDEN;

                        if ( fieldLabelWidget )
                        {
                            labelPos = field->uiLabelPosition();

                            if ( labelPos == UiItemInfo::HIDDEN )
                            {
                                fieldLabelWidget->hide();
                            }
                            else if ( labelPos == UiItemInfo::TOP )
                            {
                                QVBoxLayout* labelAndFieldVerticalLayout = new QVBoxLayout();
                                parentLayout->addLayout( labelAndFieldVerticalLayout,
                                                         currentRowIndex,
                                                         currentColumn,
                                                         1,
                                                         itemColumnSpan,
                                                         Qt::AlignTop );
                                labelAndFieldVerticalLayout->addWidget( fieldLabelWidget, 0, Qt::AlignTop );
                                labelAndFieldVerticalLayout->addWidget( fieldEditorWidget, 1, Qt::AlignTop );

                                // Apply margins determined by the editor type
                                // fieldLabelWidget->setContentsMargins(fieldEditor->labelContentMargins());
                                currentColumn += itemColumnSpan;
                            }
                            else
                            {
                                CAF_ASSERT( labelPos == UiItemInfo::LEFT );
                                int leftLabelColumnSpan = minimumLabelColumnSpan;
                                if ( currentLayout.leftLabelColumnSpan == UiOrdering::LayoutOptions::MAX_COLUMN_SPAN &&
                                     currentLayout.totalColumnSpan != UiOrdering::LayoutOptions::MAX_COLUMN_SPAN )
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign;
                                    spareColumnsToAssign = 0;
                                }
                                else if ( currentLayout.leftLabelColumnSpan == UiOrdering::LayoutOptions::MAX_COLUMN_SPAN )
                                {
                                    leftLabelColumnSpan += spareColumnsToAssign / 2;
                                    spareColumnsToAssign -= spareColumnsToAssign / 2;
                                }

                                parentLayout->addWidget( fieldLabelWidget,
                                                         currentRowIndex,
                                                         currentColumn,
                                                         1,
                                                         leftLabelColumnSpan,
                                                         Qt::AlignTop );
                                currentColumn += leftLabelColumnSpan;

                                // Apply margins determined by the editor type
                                fieldLabelWidget->setContentsMargins( fieldEditor->labelContentMargins() );
                            }
                        }

                        if ( labelPos != UiItemInfo::TOP ) // Already added if TOP
                        {
                            fieldColumnSpan += spareColumnsToAssign;

                            CAF_ASSERT( fieldColumnSpan >= 1 && "Need at least one column for the field" );
                            fieldColumnSpan = std::max( 1, fieldColumnSpan );

                            parentLayout->addWidget( fieldEditorWidget,
                                                     currentRowIndex,
                                                     currentColumn,
                                                     1,
                                                     fieldColumnSpan,
                                                     Qt::AlignTop );
                            currentColumn += fieldColumnSpan;
                        }

                        if ( previousTabOrderWidget )
                        {
                            QWidget::setTabOrder( previousTabOrderWidget, fieldEditorWidget );
                        }
                        previousTabOrderWidget = fieldEditorWidget;

                        parentLayout->setRowStretch( currentRowIndex, fieldEditor->rowStretchFactor() );
                        sumRowStretch += fieldEditor->rowStretchFactor();
                    }
                    fieldEditor->updateUi();
                }
            }
        }

        CAF_ASSERT( currentColumn <= totalColumns );
    }
    containerWidgetWithGridLayout->updateGeometry();
    // The magnitude of the stretch should not be sent up, only if there was stretch or not
    return sumRowStretch;
}

//--------------------------------------------------------------------------------------------------
/// Create a group and add widgets. Return true if the containing row needs to be stretched.
//--------------------------------------------------------------------------------------------------
int caf::UiFormLayoutObjectEditor::recursivelyAddGroupToGridLayout( UiItem*      currentItem,
                                                                    QWidget*     containerWidgetWithGridLayout,
                                                                    QGridLayout* parentLayout,
                                                                    int          currentRowIndex,
                                                                    int          currentColumn,
                                                                    int          itemColumnSpan )
{
    UiGroup* group = static_cast<UiGroup*>( currentItem );

    QMinimizePanel* groupBox = findOrCreateGroupBox( containerWidgetWithGridLayout, group );

    int stretch = recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, groupBox->contentFrame() );

    /// Insert the group box at the correct position of the parent layout
    parentLayout->addWidget( groupBox, currentRowIndex, currentColumn, 1, itemColumnSpan );

    return stretch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::UiFormLayoutObjectEditor::isUiGroupExpanded( const UiGroup* uiGroup ) const
{
    if ( uiGroup->hasForcedExpandedState() ) return uiGroup->forcedExpandedState();

    auto kwMapPair =
        m_objectKeywordGroupUiNameExpandedState.find( object()->capability<ObjectIoCapability>()->classKeyword() );
    if ( kwMapPair != m_objectKeywordGroupUiNameExpandedState.end() )
    {
        auto keyword = uiGroup->keyword();

        auto uiNameExpStatePair = kwMapPair->second.find( keyword );
        if ( uiNameExpStatePair != kwMapPair->second.end() )
        {
            return uiNameExpStatePair->second;
        }
    }

    return uiGroup->isExpandedByDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel* caf::UiFormLayoutObjectEditor::findOrCreateGroupBox( QWidget* parent, UiGroup* group )
{
    auto            groupBoxKey = group->keyword();
    QMinimizePanel* groupBox    = nullptr;

    // Find or create groupBox
    auto it = m_groupBoxes.find( groupBoxKey );

    if ( it == m_groupBoxes.end() )
    {
        groupBox = new QMinimizePanel( parent );
        groupBox->enableFrame( group->enableFrame() );
        groupBox->setTitle( QString::fromStdString( group->uiName() ) );
        groupBox->setObjectName( QString::fromStdString( group->keyword() ) );
        connect( groupBox, SIGNAL( expandedChanged( bool ) ), this, SLOT( groupBoxExpandedStateToggled( bool ) ) );

        m_newGroupBoxes[groupBoxKey] = groupBox;
    }
    else
    {
        groupBox = it->second;
        CAF_ASSERT( groupBox );
        m_newGroupBoxes[groupBoxKey] = groupBox;
    }

    QMargins contentMargins;
    if ( group->enableFrame() )
    {
        contentMargins = QMargins( 6, 6, 6, 6 );
    }

    ensureWidgetContainsEmptyGridLayout( groupBox->contentFrame(), contentMargins );

    // Set Expanded state
    bool isExpanded = isUiGroupExpanded( group );
    groupBox->setExpanded( isExpanded );

    // Update the title to be able to support dynamic group names
    groupBox->setTitle( QString::fromStdString( group->uiName() ) );
    groupBox->updateGeometry();
    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiFieldEditorHandle* caf::UiFormLayoutObjectEditor::findOrCreateFieldEditor( QWidget* parent, FieldUiCapability* field )
{
    caf::UiFieldEditorHandle* fieldEditor = nullptr;

    std::map<FieldHandle*, UiFieldEditorHandle*>::iterator it = m_fieldViews.find( field->fieldHandle() );

    if ( it == m_fieldViews.end() )
    {
        fieldEditor = UiFieldEditorHelper::createFieldEditorForField( field );

        if ( fieldEditor )
        {
            m_fieldViews[field->fieldHandle()] = fieldEditor;
            fieldEditor->setContainingEditor( this );
            fieldEditor->setUiField( field );
            fieldEditor->createWidgets( parent );
        }
        else
        {
            // This assert happens if no editor is available for a given field
            // If the macro for registering the editor is put as the single statement
            // in a cpp file, a dummy static class must be used to make sure the compile unit
            // is included
            //
            // See cafUiCoreColor3f and cafUiCoreVec3d

            // This assert will trigger for ChildArrayField and ChildField
            // Consider to exclude assert or add editors for these types if the assert is reintroduced
            // CAF_ASSERT(false);
        }
    }
    else
    {
        fieldEditor = it->second;
    }

    if ( fieldEditor )
    {
        m_usedFields.insert( field->fieldHandle() );
    }

    return fieldEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiFormLayoutObjectEditor::ensureWidgetContainsEmptyGridLayout( QWidget* containerWidget, QMargins contentMargins )
{
    CAF_ASSERT( containerWidget );
    QLayout* layout = containerWidget->layout();
    if ( layout != nullptr )
    {
        // Remove all items from the layout, then re-parent the layout to a temporary
        // This is because you cannot remove a layout from a widget but it gets moved when re-parenting.
        QLayoutItem* item;
        while ( ( item = layout->takeAt( 0 ) ) != 0 )
        {
        }
        QWidget().setLayout( layout );
    }

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setContentsMargins( contentMargins );
    containerWidget->setLayout( gridLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiFormLayoutObjectEditor::groupBoxExpandedStateToggled( bool isExpanded )
{
    if ( !this->object()->capability<ObjectIoCapability>() ) return;

    auto            objKeyword = this->object()->capability<ObjectIoCapability>()->classKeyword();
    QMinimizePanel* panel      = dynamic_cast<QMinimizePanel*>( this->sender() );

    if ( !panel ) return;

    m_objectKeywordGroupUiNameExpandedState[objKeyword][panel->objectName().toStdString()] = isExpanded;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiFormLayoutObjectEditor::cleanupBeforeSettingObject()
{
    std::map<FieldHandle*, UiFieldEditorHandle*>::iterator it;
    for ( it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it )
    {
        UiFieldEditorHandle* fvh = it->second;
        delete fvh;
    }
    m_fieldViews.clear();

    m_newGroupBoxes.clear();

    for ( auto groupIt = m_groupBoxes.begin(); groupIt != m_groupBoxes.end(); ++groupIt )
    {
        if ( !groupIt->second.isNull() ) groupIt->second->deleteLater();
    }

    m_groupBoxes.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::UiFormLayoutObjectEditor::configureAndUpdateUi()
{
    caf::UiOrdering config;
    if ( object() )
    {
        caf::ObjectUiCapability* uiObject = uiObj( object() );
        if ( uiObject )
        {
            uiObject->uiOrdering( config );
        }
    }

    // Set all fieldViews to be unvisited

    m_usedFields.clear();

    // Set all group Boxes to be unvisited
    m_newGroupBoxes.clear();

    recursivelyConfigureAndUpdateTopLevelUiOrdering( config );

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector<FieldHandle*> fvhToRemoveFromMap;
    for ( auto oldFvIt = m_fieldViews.begin(); oldFvIt != m_fieldViews.end(); ++oldFvIt )
    {
        if ( m_usedFields.count( oldFvIt->first ) == 0 )
        {
            // The old field editor is not present anymore, get rid of it
            delete oldFvIt->second;
            fvhToRemoveFromMap.push_back( oldFvIt->first );
        }
    }

    for ( size_t i = 0; i < fvhToRemoveFromMap.size(); ++i )
    {
        m_fieldViews.erase( fvhToRemoveFromMap[i] );
    }

    // Remove all unmentioned group boxes
    for ( auto itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld )
    {
        auto itNew = m_newGroupBoxes.find( itOld->first );
        if ( itNew == m_newGroupBoxes.end() )
        {
            // The old groupBox is not present anymore, get rid of it
            if ( !itOld->second.isNull() ) delete itOld->second;
        }
    }
    m_groupBoxes = m_newGroupBoxes;

    // Notify object when widgets have been created
    caf::ObjectUiCapability* uiObject = uiObj( object() );
    if ( uiObject )
    {
        uiObject->onEditorWidgetsCreated();
    }
}
