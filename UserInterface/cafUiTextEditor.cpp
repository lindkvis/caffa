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

#include "cafUiTextEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include <QIntValidator>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

namespace caffa
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiTextEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TextEdit::TextEdit( QWidget* parent /*= 0*/ )
    : QTextEdit( parent )
    , m_heightHint( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize TextEdit::sizeHint() const
{
    QSize mySize = QTextEdit::sizeHint();

    if ( m_heightHint > 0 )
    {
        mySize.setHeight( m_heightHint );
    }

    return mySize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEdit::setHeightHint( int heightHint )
{
    m_heightHint = heightHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TextEdit::focusOutEvent( QFocusEvent* e )
{
    QTextEdit::focusOutEvent( e );

    emit editingFinished();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTextEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_textEdit.isNull() );
    CAF_ASSERT( !m_label.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_textEdit->setReadOnly( uiField()->isUiReadOnly() );
    // m_textEdit->setEnabled(!field()->isUiReadOnly()); // Neccesary ?
    m_textEdit->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    UiTextEditorAttribute leab;

    caffa::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &leab );
    }

    m_textMode = leab.textMode;

    if ( leab.showSaveButton )
    {
        disconnect( m_textEdit, SIGNAL( editingFinished() ), this, SLOT( slotSetValueToField() ) );
        m_saveButton->show();
    }
    else
    {
        connect( m_textEdit, SIGNAL( editingFinished() ), this, SLOT( slotSetValueToField() ) );
        m_saveButton->hide();
    }

    m_textEdit->blockSignals( true );
    switch ( leab.textMode )
    {
        case UiTextEditorAttribute::PLAIN:
            m_textEdit->setPlainText( QString::fromStdString( uiField()->uiValue().value<std::string>() ) );
            break;
        case UiTextEditorAttribute::HTML:
            m_textEdit->setHtml( QString::fromStdString( uiField()->uiValue().value<std::string>() ) );
            break;
    }
    m_textEdit->blockSignals( false );

    m_textEdit->setWordWrapMode( toQTextOptionWrapMode( leab.wrapMode ) );

    m_textEdit->setFont( leab.font );
    if ( leab.heightHint > 0 )
    {
        m_textEdit->setHeightHint( leab.heightHint );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTextEditor::isMultiRowEditor() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTextEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    m_textEdit = new TextEdit( containerWidget );
    connect( m_textEdit, SIGNAL( editingFinished() ), this, SLOT( slotSetValueToField() ) );

    m_saveButton = new QPushButton( "Save changes", containerWidget );
    connect( m_saveButton, SIGNAL( clicked() ), this, SLOT( slotSetValueToField() ) );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( m_textEdit );
    layout->setMargin( 0 );

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->insertStretch( 0, 10 );
    buttonLayout->addWidget( m_saveButton );

    layout->addLayout( buttonLayout );
    containerWidget->setLayout( layout );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTextEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTextEditor::slotSetValueToField()
{
    QString textValue;

    switch ( m_textMode )
    {
        case UiTextEditorAttribute::PLAIN:
            textValue = m_textEdit->toPlainText();
            break;
        case UiTextEditorAttribute::HTML:
            textValue = m_textEdit->toHtml();
            break;
    }

    Variant v( textValue.toStdString() );

    this->setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextOption::WrapMode UiTextEditor::toQTextOptionWrapMode( UiTextEditorAttribute::WrapMode wrapMode )
{
    switch ( wrapMode )
    {
        case UiTextEditorAttribute::NoWrap:
            return QTextOption::WrapMode::NoWrap;
        case UiTextEditorAttribute::WordWrap:
            return QTextOption::WrapMode::WordWrap;
        case UiTextEditorAttribute::ManualWrap:
            return QTextOption::WrapMode::ManualWrap;
        case UiTextEditorAttribute::WrapAnywhere:
            return QTextOption::WrapMode::WrapAnywhere;
        case UiTextEditorAttribute::WrapAtWordBoundaryOrAnywhere:
        default:
            return QTextOption::WrapMode::WrapAtWordBoundaryOrAnywhere;
    }
}

} // end namespace caffa
