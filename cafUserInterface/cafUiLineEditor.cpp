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

#include "cafUiLineEditor.h"

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafSelectionManager.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QApplication>
#include <QCompleter>
#include <QDebug>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>
#include <QStringListModel>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiLineEditor::createEditorWidget( QWidget* parent )
{
    m_lineEdit = new UiLineEdit( parent );

    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    return m_lineEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiLineEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiLineEditor::configureAndUpdateUi()
{
    if ( !m_label.isNull() )
    {
        UiFieldEditorHandle::updateLabelFromField( m_label );
    }

    if ( !m_lineEdit.isNull() )
    {
        bool isReadOnly = uiField()->isUiReadOnly();
        if ( isReadOnly )
        {
            m_lineEdit->setReadOnly( true );

            m_lineEdit->setStyleSheet( "QLineEdit {"
                                       "color: #808080;"
                                       "background-color: #F0F0F0;}" );
        }
        else
        {
            m_lineEdit->setReadOnly( false );
            m_lineEdit->setStyleSheet( "" );
        }

        m_lineEdit->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

        UiLineEditorAttribute leab;
        {
            caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
            if ( uiObject )
            {
                uiObject->editorAttribute( uiField()->fieldHandle(), &leab );
            }

            if ( leab.validator )
            {
                m_lineEdit->setValidator( leab.validator );
            }

            m_lineEdit->setAvoidSendingEnterEventToParentWidget( leab.avoidSendingEnterEventToParentWidget );

            if ( leab.maximumWidth != -1 )
            {
                m_lineEdit->setMaximumWidth( leab.maximumWidth );
            }

            if ( !leab.placeholderText.empty() )
            {
                m_lineEdit->setPlaceholderText( QString::fromStdString( leab.placeholderText ) );
            }
        }

        bool fromMenuOnly = true;
        m_optionCache     = uiField()->valueOptions( &fromMenuOnly );
        CAF_ASSERT( fromMenuOnly ); // Not supported

        if ( !m_optionCache.empty() && fromMenuOnly == true )
        {
            if ( !m_completer )
            {
                m_completer         = new QCompleter( this );
                m_completerTextList = new QStringListModel( this );

                m_completer->setModel( m_completerTextList );
                m_completer->setFilterMode( leab.completerFilterMode );
                m_completer->setCaseSensitivity( leab.completerCaseSensitivity );

                m_lineEdit->setCompleter( m_completer );
                connect( m_completer,
                         SIGNAL( activated( const QModelIndex& ) ),
                         this,
                         SLOT( slotCompleterActivated( const QModelIndex& ) ) );
                m_completer->popup()->installEventFilter( this );
            }

            QStringList optionNames;
            for ( const OptionItemInfo& item : m_optionCache )
            {
                optionNames.push_back( QString::fromStdString( item.optionUiText() ) );
            }

            m_completerTextList->setStringList( optionNames );

            int enumValue = uiField()->uiValue().value<int>();

            if ( enumValue < m_optionCache.size() && enumValue > -1 )
            {
                m_lineEdit->setText( QString::fromStdString( m_optionCache[enumValue].optionUiText() ) );
            }
        }
        else
        {
            m_lineEdit->setCompleter( nullptr );
            delete m_completerTextList;
            delete m_completer;
            m_optionCache.clear();

            UiLineEditorAttributeUiDisplayString displayStringAttrib;
            caf::ObjectUiCapability*             uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
            if ( uiObject )
            {
                uiObject->editorAttribute( uiField()->fieldHandle(), &displayStringAttrib );
            }

            QString displayString;
            if ( displayStringAttrib.m_displayString.empty() )
            {
                displayString = QString::fromStdString( uiField()->uiValue().textValue() );
            }
            else
            {
                displayString = QString::fromStdString( displayStringAttrib.m_displayString );
            }

            m_lineEdit->setText( displayString );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins UiLineEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_lineEdit->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int   heightDiff = editorSize.height() - labelSize.height();

    QMargins contentMargins = m_label->contentsMargins();
    if ( heightDiff > 0 )
    {
        contentMargins.setTop( contentMargins.top() + heightDiff / 2 );
        contentMargins.setBottom( contentMargins.bottom() + heightDiff / 2 );
    }
    return contentMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiLineEditor::slotEditingFinished()
{
    if ( !m_optionCache.empty() )
    {
        auto item = findOption( m_lineEdit->text() );
        if ( item )
        {
            this->setValueToField( item->value() );
        }
        else
        {
            // Try to complete the text in the widget

            QModelIndex sourceindex =
                static_cast<QAbstractProxyModel*>( m_completer->completionModel() )->mapToSource( m_completer->currentIndex() );

            if ( sourceindex.isValid() )
            {
                int currentRow = sourceindex.row();
                {
                    // If the existing value in the field is the same as the completer will hit, we need to echo the
                    // choice into the text field because the field values are equal, so the normal echoing is
                    // considered unnecessary by the caf system.
                    Variant currentFieldValue = uiField()->uiValue();
                    if ( m_optionCache[currentRow].value() == currentFieldValue )
                    {
                        m_lineEdit->setText(
                            m_completer->completionModel()->data( m_completer->currentIndex() ).toString() );
                    }
                }

                this->setValueToField( m_optionCache[currentRow].value() );
            }
            else
            {
                // Revert to value stored in the Field, because we didn't find any matches
                this->updateUi();
            }
        }
    }
    else
    {
        QString textValue = m_lineEdit->text();
        this->setValueToField( Variant( textValue.toStdString() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiLineEditor::isMultipleFieldsWithSameKeywordSelected( FieldHandle* editorField ) const
{
    std::vector<FieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back( editorField );

    // For current selection, find all fields with same keyword
    std::vector<UiItem*> items;
    SelectionManager::instance()->selectedItems( items, SelectionManager::FIRST_LEVEL );

    for ( size_t i = 0; i < items.size(); i++ )
    {
        FieldUiCapability* uiField = dynamic_cast<FieldUiCapability*>( items[i] );
        if ( !uiField ) continue;

        FieldHandle* field = uiField->fieldHandle();
        if ( field && field != editorField && field->keyword() == editorField->keyword() )
        {
            fieldsToUpdate.push_back( field );
        }
    }

    if ( fieldsToUpdate.size() > 1 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiLineEdit::UiLineEdit( QWidget* parent )
    : QLineEdit( parent )
    , m_avoidSendingEnterEvent( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiLineEdit::setAvoidSendingEnterEventToParentWidget( bool avoidSendingEnterEvent )
{
    m_avoidSendingEnterEvent = avoidSendingEnterEvent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiLineEdit::keyPressEvent( QKeyEvent* event )
{
    QLineEdit::keyPressEvent( event );
    if ( m_avoidSendingEnterEvent )
    {
        if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        {
            // accept enter/return events so they won't
            // be ever propagated to the parent dialog..
            event->accept();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Event filter filtering events to the QCompleter
//--------------------------------------------------------------------------------------------------
bool UiLineEditor::eventFilter( QObject* watched, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>( event );

        if ( ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter )
        {
            m_ignoreCompleterActivated = true;
            this->m_completer->popup()->close();
            this->slotEditingFinished();
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiLineEditor::slotCompleterActivated( const QModelIndex& index )
{
    if ( m_completer && !m_ignoreCompleterActivated )
    {
        slotEditingFinished();
    }

    m_ignoreCompleterActivated = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const OptionItemInfo* UiLineEditor::findOption( const QString& uiText )
{
    QString uiTextTrimmed = uiText.trimmed();
    for ( int idx = 0; idx < m_optionCache.size(); ++idx )
    {
        if ( uiTextTrimmed.toStdString() == m_optionCache[idx].optionUiText() )
        {
            return &m_optionCache[idx];
        }
    }

    return nullptr;
}

// Define at this location to avoid duplicate symbol definitions in 'cafUiDefaultObjectEditor.cpp' in a cotire build.
// The variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same
// line number.
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiLineEditor );

} // end namespace caf
