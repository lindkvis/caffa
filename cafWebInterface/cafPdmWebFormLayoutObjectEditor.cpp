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

#include "cafPdmWebFormLayoutObjectEditor.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmObjectXmlCapability.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmWebComboBoxEditor.h"
#include "cafPdmWebFieldEditorHandle.h"

#include "cafAssert.h"

#include "Wt/WContainerWidget.h"
#include "Wt/WGridLayout.h"
#include "Wt/WIconPair.h"
#include "Wt/WPanel.h"
#include "Wt/WVBoxLayout.h"
#include "Wt/WWidget.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmWebFormLayoutObjectEditor::PdmWebFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmWebFormLayoutObjectEditor::~PdmWebFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmWebFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInGridLayout( const PdmUiOrdering& uiOrdering,
                                                                                             Wt::WGridLayout* parentLayout,
                                                                                             const QString& uiConfigName )
{
    std::list<std::unique_ptr<Wt::WWidget>> existingWidgets;
    parentLayout->iterateWidgets( [&]( Wt::WWidget* widget ) {
        existingWidgets.push_back( std::move( parentLayout->removeWidget( widget ) ) );
    } );

    for ( PdmUiItem* item : uiOrdering.uiItems() )
    {
        if ( item->isUiGroup() )
        {
            auto group = recursivelyCreateGroup( existingWidgets, item, uiConfigName );
        }
        PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*>( item );
        if ( !field ) continue;

        PdmWebFieldEditorHandle* fieldEditor = findOrCreateFieldEditor( field, uiConfigName );
        if ( !fieldEditor ) continue;

        existingWidgets.push_back( fieldEditor->findOrCreateLabelWidget( existingWidgets ) );
        existingWidgets.push_back( fieldEditor->findOrCreateEditorWidget( existingWidgets ) );
        fieldEditor->updateUi( uiConfigName );
    }

    for ( int currentRowIndex = 0; currentRowIndex < (int)uiOrdering.uiItems().size(); ++currentRowIndex )
    {
        PdmUiItem* currentItem = uiOrdering.uiItems()[currentRowIndex];

        if ( currentItem->isUiGroup() )
        {
            auto group = recursivelyCreateGroup( existingWidgets, currentItem, uiConfigName );

            /// Insert the group box at the correct position of the parent layout
            parentLayout->addWidget( std::move( group ), currentRowIndex, 0, 1, 2 );
        }
        else
        {
            PdmWebFieldEditorHandle* fieldEditor = nullptr;
            PdmUiFieldHandle*        field       = dynamic_cast<PdmUiFieldHandle*>( currentItem );

            if ( field ) fieldEditor = findOrCreateFieldEditor( field, uiConfigName );

            if ( fieldEditor )
            {
                auto fieldEditorWidget = fieldEditor->findOrCreateEditorWidget( existingWidgets );
                if ( !fieldEditorWidget ) continue;

                auto                        fieldLabelWidget = fieldEditor->findOrCreateLabelWidget( existingWidgets );
                PdmUiItemInfo::LabelPosType labelPos         = PdmUiItemInfo::HIDDEN;

                if ( fieldLabelWidget )
                {
                    labelPos = field->uiLabelPosition( uiConfigName );

                    CAF_ASSERT( labelPos == PdmUiItemInfo::LEFT );

                    Wt::WString labelString = fieldLabelWidget->text();
                    parentLayout->addWidget( std::move( fieldLabelWidget ),
                                             currentRowIndex,
                                             0,
                                             1,
                                             1,
                                             Wt::AlignmentFlag::Middle | Wt::AlignmentFlag::Left );
                    parentLayout->setColumnStretch( 0, 1 );
                }

                auto editorMinWidth = fieldEditorWidget->minimumWidth();
                parentLayout->addWidget( std::move( fieldEditorWidget ),
                                         currentRowIndex,
                                         1,
                                         1,
                                         1,
                                         Wt::AlignmentFlag::Middle | Wt::AlignmentFlag::Left );
                parentLayout->setColumnStretch( 1, 1 );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WPanel>
    caf::PdmWebFormLayoutObjectEditor::recursivelyCreateGroup( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets,
                                                               PdmUiItem*                               currentItem,
                                                               const QString&                           uiConfigName )
{
    PdmUiGroup* group = static_cast<PdmUiGroup*>( currentItem );

    std::unique_ptr<Wt::WPanel> groupBox;
    for ( std::unique_ptr<Wt::WWidget>& widgetPtr : existingWidgets )
    {
        if ( dynamic_cast<Wt::WPanel*>( widgetPtr.get() ) && widgetPtr->objectName() == group->keyword().toStdString() )
        {
            groupBox = std::unique_ptr<Wt::WPanel>( static_cast<Wt::WPanel*>( widgetPtr.release() ) );
            return groupBox;
        }
    }
    if ( !groupBox )
    {
        groupBox = createGroupBox( group, uiConfigName );
    }

    groupBox->setTitle( group->uiName( uiConfigName ).toStdString() );
    auto centralWidget = dynamic_cast<Wt::WContainerWidget*>( groupBox->centralWidget() );
    CAF_ASSERT( centralWidget );
    auto gridLayout = dynamic_cast<Wt::WGridLayout*>( centralWidget->layout() );
    CAF_ASSERT( gridLayout );
    recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, gridLayout, uiConfigName );

    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WPanel> caf::PdmWebFormLayoutObjectEditor::createGroupBox( PdmUiGroup*    group,
                                                                               const QString& uiConfigName )
{
    auto groupBox = std::make_unique<Wt::WPanel>();
    groupBox->setMargin( 0 );
    groupBox->setCollapsible( true );
    groupBox->collapseIcon()->setFloatSide( Wt::Side::Right );
    groupBox->setTitle( group->uiName( uiConfigName ).toStdString() );
    groupBox->setObjectName( group->keyword().toStdString() );
    auto centralWidget = groupBox->setCentralWidget( std::make_unique<Wt::WContainerWidget>() );
    centralWidget->setMargin( 0 );
    centralWidget->setPadding( 0 );
    auto gridLayout = std::make_unique<Wt::WGridLayout>();
    auto layout     = centralWidget->setLayout( std::move( gridLayout ) );
    layout->setVerticalSpacing( 4 );
    layout->setContentsMargins( 0, 0, 0, 18 );

    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmWebFieldEditorHandle* caf::PdmWebFormLayoutObjectEditor::findOrCreateFieldEditor( PdmUiFieldHandle* field,
                                                                                          const QString& uiConfigName )
{
    caf::PdmWebFieldEditorHandle* fieldEditor = nullptr;

    std::map<PdmFieldHandle*, PdmWebFieldEditorHandle*>::iterator it = m_fieldViews.find( field->fieldHandle() );

    if ( it == m_fieldViews.end() )
    {
        // If editor type is specified, find in factory
        if ( !field->uiEditorTypeName( uiConfigName ).isEmpty() )
        {
            fieldEditor = caf::Factory<PdmWebFieldEditorHandle, QString>::instance()->create(
                field->uiEditorTypeName( uiConfigName ) );
        }
        else
        {
            // Find the default field editor
            QString fieldTypeName = qStringTypeName( *( field->fieldHandle() ) );
            if ( field->toUiBasedQVariant().type() != QVariant::List )
            {
                // Handle a single value field with valueOptions: Make a combobox

                bool                     useOptionsOnly = true;
                QList<PdmOptionItemInfo> options        = field->valueOptions( &useOptionsOnly );
                CAF_ASSERT( useOptionsOnly ); // Not supported

                if ( !options.empty() )
                {
                    fieldTypeName = caf::PdmWebComboBoxEditor::uiEditorTypeName();
                }
            }

            fieldEditor = caf::Factory<PdmWebFieldEditorHandle, QString>::instance()->create( fieldTypeName );
        }
        CAF_ASSERT( fieldEditor );
        fieldEditor->setContainingEditor( this );
        fieldEditor->setUiField( field );
        m_fieldViews[field->fieldHandle()] = fieldEditor;
    }
    else
    {
        fieldEditor = it->second;
    }

    return fieldEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmWebFormLayoutObjectEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    caf::PdmUiOrdering config;
    if ( pdmObject() )
    {
        caf::PdmUiObjectHandle* uiObject = uiObj( pdmObject() );
        if ( uiObject )
        {
            uiObject->uiOrdering( uiConfigName, config );
        }
    }
    recursivelyConfigureAndUpdateTopLevelUiOrdering( config, uiConfigName );

    // Notify pdm object when widgets have been created
    caf::PdmUiObjectHandle* uiObject = uiObj( pdmObject() );
    if ( uiObject )
    {
        uiObject->onEditorWidgetsCreated();
    }
}
