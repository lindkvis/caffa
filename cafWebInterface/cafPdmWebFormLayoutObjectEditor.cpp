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

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"
#include "cafObjectUiCapability.h"
#include "cafPdmWebComboBoxEditor.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafAssert.h"

#include "Wt/WContainerWidget.h"
#include "Wt/WGridLayout.h"
#include "Wt/WIconPair.h"
#include "Wt/WPanel.h"
#include "Wt/WVBoxLayout.h"
#include "Wt/WWidget.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebFormLayoutObjectEditor::PdmWebFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebFormLayoutObjectEditor::~PdmWebFormLayoutObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFormLayoutObjectEditor::recursivelyConfigureAndUpdateUiOrderingInGridLayout( const UiOrdering& uiOrdering,
                                                                                        Wt::WGridLayout*  parentLayout )
{
    std::list<std::unique_ptr<Wt::WWidget>> existingWidgets;
    parentLayout->iterateWidgets( [&]( Wt::WWidget* widget ) {
        existingWidgets.push_back( std::move( parentLayout->removeWidget( widget ) ) );
    } );

    for ( UiItem* item : uiOrdering.uiItems() )
    {
        if ( item->isUiGroup() )
        {
            auto group = recursivelyCreateGroup( existingWidgets, item );
        }
        FieldUiCapability* field = dynamic_cast<FieldUiCapability*>( item );
        if ( !field ) continue;

        PdmWebFieldEditorHandle* fieldEditor = findOrCreateFieldEditor( field );
        if ( !fieldEditor ) continue;

        existingWidgets.push_back( fieldEditor->findOrCreateLabelWidget( existingWidgets ) );
        existingWidgets.push_back( fieldEditor->findOrCreateEditorWidget( existingWidgets ) );
        fieldEditor->updateUi();
    }

    for ( int currentRowIndex = 0; currentRowIndex < (int)uiOrdering.uiItems().size(); ++currentRowIndex )
    {
        UiItem* currentItem = uiOrdering.uiItems()[currentRowIndex];

        if ( currentItem->isUiGroup() )
        {
            auto group = recursivelyCreateGroup( existingWidgets, currentItem );

            /// Insert the group box at the correct position of the parent layout
            parentLayout->addWidget( std::move( group ), currentRowIndex, 0, 1, 2 );
        }
        else
        {
            PdmWebFieldEditorHandle* fieldEditor = nullptr;
            FieldUiCapability*       field       = dynamic_cast<FieldUiCapability*>( currentItem );

            if ( field ) fieldEditor = findOrCreateFieldEditor( field );

            if ( fieldEditor )
            {
                auto fieldEditorWidget = fieldEditor->findOrCreateEditorWidget( existingWidgets );
                if ( !fieldEditorWidget ) continue;

                auto                     fieldLabelWidget = fieldEditor->findOrCreateLabelWidget( existingWidgets );
                UiItemInfo::LabelPosType labelPos         = UiItemInfo::HIDDEN;

                if ( fieldLabelWidget )
                {
                    labelPos = field->uiLabelPosition();

                    CAF_ASSERT( labelPos == UiItemInfo::LEFT );

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
    PdmWebFormLayoutObjectEditor::recursivelyCreateGroup( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets,
                                                          UiItem*                                  currentItem )
{
    UiGroup* group = static_cast<UiGroup*>( currentItem );

    std::unique_ptr<Wt::WPanel> groupBox;
    for ( std::unique_ptr<Wt::WWidget>& widgetPtr : existingWidgets )
    {
        if ( dynamic_cast<Wt::WPanel*>( widgetPtr.get() ) && widgetPtr->objectName() == group->keyword() )
        {
            groupBox = std::unique_ptr<Wt::WPanel>( static_cast<Wt::WPanel*>( widgetPtr.release() ) );
            return groupBox;
        }
    }
    if ( !groupBox )
    {
        groupBox = createGroupBox( group );
    }

    groupBox->setTitle( group->uiName() );
    auto centralWidget = dynamic_cast<Wt::WContainerWidget*>( groupBox->centralWidget() );
    CAF_ASSERT( centralWidget );
    auto gridLayout = dynamic_cast<Wt::WGridLayout*>( centralWidget->layout() );
    CAF_ASSERT( gridLayout );
    recursivelyConfigureAndUpdateUiOrderingInGridLayout( *group, gridLayout );

    return groupBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<Wt::WPanel> PdmWebFormLayoutObjectEditor::createGroupBox( UiGroup* group )
{
    auto groupBox = std::make_unique<Wt::WPanel>();
    groupBox->setMargin( 0 );
    groupBox->setCollapsible( true );
    groupBox->collapseIcon()->setFloatSide( Wt::Side::Right );
    groupBox->setTitle( group->uiName() );
    groupBox->setObjectName( group->keyword() );
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
PdmWebFieldEditorHandle* PdmWebFormLayoutObjectEditor::findOrCreateFieldEditor( FieldUiCapability* field )
{
    PdmWebFieldEditorHandle* fieldEditor = nullptr;

    std::map<FieldHandle*, PdmWebFieldEditorHandle*>::iterator it = m_fieldViews.find( field->fieldHandle() );

    if ( it == m_fieldViews.end() )
    {
        // If editor type is specified, find in factory
        if ( !field->uiEditorTypeName().empty() )
        {
            fieldEditor = Factory<PdmWebFieldEditorHandle, std::string>::instance()->create( field->uiEditorTypeName() );
        }
        else
        {
            // Find the default field editor
            std::string fieldTypeName = typeid( *( field->fieldHandle() ) ).name();
            if ( !field->toUiBasedVariant().isVector() )
            {
                // Handle a single value field with valueOptions: Make a combobox

                bool                          useOptionsOnly = true;
                std::deque<OptionItemInfo> options        = field->valueOptions( &useOptionsOnly );
                CAF_ASSERT( useOptionsOnly ); // Not supported

                if ( !options.empty() )
                {
                    fieldTypeName = PdmWebComboBoxEditor::uiEditorTypeName();
                }
            }

            fieldEditor = Factory<PdmWebFieldEditorHandle, std::string>::instance()->create( fieldTypeName );
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
void PdmWebFormLayoutObjectEditor::configureAndUpdateUi()
{
    UiOrdering config;
    if ( pdmObject() )
    {
        ObjectUiCapability* uiObject = uiObj( pdmObject() );
        if ( uiObject )
        {
            uiObject->uiOrdering( config );
        }
    }
    recursivelyConfigureAndUpdateTopLevelUiOrdering( config );

    // Notify pdm object when widgets have been created
    ObjectUiCapability* uiObject = uiObj( pdmObject() );
    if ( uiObject )
    {
        uiObject->onEditorWidgetsCreated();
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
