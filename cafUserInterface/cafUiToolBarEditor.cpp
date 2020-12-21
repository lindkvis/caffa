//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafUiToolBarEditor.h"

#include "cafField.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"
#include "cafUiComboBoxEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiFieldEditorHelper.h"
#include "cafUiLineEditor.h"
#include "cafUiOrdering.h"
#include "cafUiPushButtonEditor.h"
#include "cafUiToolButtonEditor.h"

#include <QAction>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::PdmUiToolBarEditor( const QString& title, QMainWindow* mainWindow )
{
    m_toolbar = new QToolBar( mainWindow );
    m_toolbar->setObjectName( title );
    m_toolbar->setWindowTitle( title );

    mainWindow->addToolBar( m_toolbar );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::~PdmUiToolBarEditor()
{
    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiToolBarEditor::isEditorDataValid( const std::vector<caf::FieldHandle*>& fields ) const
{
    if ( m_fields.size() == fields.size() && m_fieldViews.size() == fields.size() )
    {
        bool equalContent = true;

        for ( size_t i = 0; i < m_fields.size(); i++ )
        {
            if ( m_fields[i] != fields[i] )
            {
                equalContent = false;
            }
        }

        if ( equalContent )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::configureAndUpdateUi()
{
    {
        // Find set of owner objects. Can be several objects, make a set to avoid calling uiOrdering more than once for
        // an object

        std::set<caf::ObjectUiCapability*> ownerUiObjects;

        for ( FieldHandle* field : m_fields )
        {
            caf::ObjectUiCapability* ownerUiObject = field->ownerObject()->capability<ObjectUiCapability>();
            if ( ownerUiObject )
            {
                ownerUiObjects.insert( ownerUiObject );
            }
        }

        UiOrdering config;
        for ( caf::ObjectUiCapability* ownerUiObject : ownerUiObjects )
        {
            ownerUiObject->uiOrdering( config );
        }
    }

    for ( FieldHandle* field : m_fields )
    {
        UiFieldEditorHandle* fieldEditor = nullptr;

        // Find or create FieldEditor
        std::map<std::string, UiFieldEditorHandle*>::iterator it;
        it = m_fieldViews.find( field->keyword() );
        if ( it == m_fieldViews.end() )
        {
            caf::FieldUiCapability* uiFieldHandle = field->capability<FieldUiCapability>();

            bool addSpace = false;
            if ( uiFieldHandle )
            {
                if ( uiFieldHandle->uiValue().canConvert<bool>() )
                {
                    auto editorTypeName = caf::PdmUiToolButtonEditor::uiEditorTypeName();

                    fieldEditor = caf::Factory<UiFieldEditorHandle, std::string>::instance()->create( editorTypeName );
                }
                else
                {
                    fieldEditor =
                        caf::UiFieldEditorHelper::createFieldEditorForField( field->capability<FieldUiCapability>() );

                    addSpace = true;
                }
            }

            if ( fieldEditor )
            {
                m_fieldViews[field->keyword()] = fieldEditor;
                fieldEditor->setUiField( uiFieldHandle );
                fieldEditor->createWidgets( nullptr );
                m_actions.push_back( m_toolbar->addWidget( fieldEditor->editorWidget() ) );

                if ( addSpace )
                {
                    QWidget* widget = new QWidget;
                    widget->setMinimumWidth( 5 );
                    m_toolbar->addWidget( widget );
                }

                fieldEditor->updateUi();
            }
        }
        else
        {
            if ( it->second )
            {
                it->second->updateUi();
            }
        }
    }

    CAF_ASSERT( m_fields.size() == m_fieldViews.size() );
    CAF_ASSERT( static_cast<int>( m_fields.size() ) == m_actions.size() );

    for ( size_t i = 0; i < m_fields.size(); i++ )
    {
        caf::FieldHandle* field = m_fields[i];

        // Enabled state of a tool button is controlled by the QAction associated with a tool button
        // Changing the state of a widget directly has no effect
        // See Qt doc for QToolBar::insertWidget
        QAction* action = m_actions[static_cast<int>( i )];

        caf::FieldUiCapability* uiFieldHandle = field->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            action->setEnabled( !uiFieldHandle->isUiReadOnly() );
        }

        // TODO: Show/hide of tool bar items can be done by
        // action->setVisible(!field->isUiHidden());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiToolBarEditor::focusWidget( UiFieldEditorHandle* uiFieldEditorHandle )
{
    // Some editors have a placeholder widget as parent of the main editor widget
    // This is the case for combo box widget to allow up/down arrow buttons associated with the combo box

    QWidget* editorWidget = nullptr;
    auto     comboEditor  = dynamic_cast<caf::PdmUiComboBoxEditor*>( uiFieldEditorHandle );
    if ( comboEditor )
    {
        auto       topWidget = comboEditor->editorWidget();
        QComboBox* comboBox  = topWidget->findChild<QComboBox*>();

        editorWidget = comboBox;
    }
    else
    {
        editorWidget = uiFieldEditorHandle->editorWidget();
    }

    return editorWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::setFields( std::vector<caf::FieldHandle*>& fields )
{
    clear();

    m_fields = fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::clear()
{
    for ( const auto& it : m_fieldViews )
    {
        delete it.second;
    }

    m_fieldViews.clear();

    if ( m_toolbar )
    {
        m_toolbar->clear();
    }

    m_actions.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::setFocusWidgetFromKeyword( const std::string& fieldKeyword )
{
    if ( !m_toolbar->isVisible() ) return;

    auto fieldView = m_fieldViews.find( fieldKeyword );
    if ( fieldView != m_fieldViews.end() && fieldView->second )
    {
        QWidget* widget = focusWidget( fieldView->second );

        if ( widget )
        {
            widget->setFocus( Qt::ActiveWindowFocusReason );

            PdmUiLineEditorAttribute attributes;

            for ( auto field : m_fields )
            {
                if ( field->keyword() == fieldKeyword )
                {
                    caf::ObjectUiCapability* uiObject = uiObj( field->ownerObject() );
                    if ( uiObject )
                    {
                        uiObject->editorAttribute( field, &attributes );
                    }
                }
            }

            if ( attributes.selectAllOnFocusEvent )
            {
                auto lineEdit = dynamic_cast<QLineEdit*>( widget );
                if ( lineEdit )
                {
                    lineEdit->selectAll();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::show()
{
    if ( m_toolbar )
    {
        m_toolbar->show();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::hide()
{
    if ( m_toolbar )
    {
        m_toolbar->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string PdmUiToolBarEditor::keywordForFocusWidget()
{
    std::string keyword;

    if ( m_toolbar->isVisible() )
    {
        for ( auto fieldViewPair : m_fieldViews )
        {
            auto uiFieldEditorHandle = fieldViewPair.second;
            if ( uiFieldEditorHandle )
            {
                auto widget = focusWidget( uiFieldEditorHandle );
                if ( widget && widget->hasFocus() )
                {
                    keyword = fieldViewPair.first;
                }
            }
        }
    }

    return keyword;
}

} // end namespace caf
