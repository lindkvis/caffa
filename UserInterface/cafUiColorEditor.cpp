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

#include "cafUiColorEditor.h"

#include "cafUiDefaultObjectEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafFactory.h"

#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

namespace caffa
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiColorEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiColorEditor::UiColorEditor()
{
    m_color = QColor::Invalid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiColorEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_label.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    caffa::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );

        if ( m_attributes.showLabel )
        {
            m_colorTextLabel->show();
        }
        else
        {
            m_colorTextLabel->hide();
        }
    }

    caffa::Color color = uiField()->uiValue().value<caffa::Color>();
    setColorOnWidget( color.to<QColor>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins UiColorEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_colorSelectionButton->sizeHint();
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
QWidget* UiColorEditor::createEditorWidget( QWidget* parent )
{
    QWidget*     placeholder = new QWidget( parent );
    QHBoxLayout* layout      = new QHBoxLayout( placeholder );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    m_colorTextLabel = new QLabel( parent );

    m_colorSelectionButton = new QToolButton( parent );
    m_colorSelectionButton->setObjectName( "ColorSelectionButton" );
    m_colorSelectionButton->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_colorSelectionButton->setLayout( buttonLayout );
    QMargins buttonMargins( 3, 3, 3, 3 );
    buttonLayout->setContentsMargins( buttonMargins );

    m_colorPreviewLabel = new QLabel( m_colorSelectionButton );
    m_colorPreviewLabel->setObjectName( "ColorPreviewLabel" );
    m_colorPreviewLabel->setText( QLatin1String( "..." ) );
    m_colorPreviewLabel->setAlignment( Qt::AlignCenter );

    QFontMetrics fontMetrics = QApplication::fontMetrics();

    buttonLayout->addWidget( m_colorPreviewLabel );
    m_colorSelectionButton->setMinimumWidth( fontMetrics.boundingRect( m_colorPreviewLabel->text() ).width() + 15 );

    layout->addWidget( m_colorTextLabel );
    layout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored ) );
    layout->addWidget( m_colorSelectionButton );

    connect( m_colorSelectionButton, SIGNAL( clicked() ), this, SLOT( colorSelectionClicked() ) );

    return placeholder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiColorEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiColorEditor::colorSelectionClicked()
{
    QColorDialog::ColorDialogOptions flags;
    if ( m_attributes.showAlpha )
    {
        flags |= QColorDialog::ShowAlphaChannel;
    }

    QColor newColor = QColorDialog::getColor( m_color, m_colorSelectionButton, "Select color", flags );
    if ( newColor.isValid() && newColor != m_color )
    {
        setColorOnWidget( newColor );
        Variant v( caffa::Color( newColor.red(), newColor.green(), newColor.blue() ) );
        this->setValueToField( v );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiColorEditor::setColorOnWidget( const QColor& color )
{
    if ( m_color != color )
    {
        m_color = color;

        QString colorString;
        if ( !color.isValid() )
        {
            colorString = "Undefined";
            m_colorSelectionButton->setStyleSheet( "" );
        }
        else
        {
            QColor  fontColor = getFontColor( m_color );
            QString styleTemplate =
                "QLabel#ColorPreviewLabel { background-color: %1; color: %2; border: 1px solid black; }";
            QString styleSheet = QString( styleTemplate ).arg( m_color.name() ).arg( fontColor.name() );

            m_colorPreviewLabel->setStyleSheet( styleSheet );
            colorString = QString( "[%1, %2, %3]" )
                              .arg( QString::number( color.red() ) )
                              .arg( QString::number( color.green() ) )
                              .arg( QString::number( color.blue() ) );

            if ( m_attributes.showAlpha )
            {
                colorString += QString( " (%4)" ).arg( QString::number( color.alpha() ) );
            }
        }
        if ( m_attributes.showLabel )
        {
            m_colorTextLabel->setText( colorString );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Based on http://www.codeproject.com/cs/media/IdealTextColor.asp
//--------------------------------------------------------------------------------------------------
QColor UiColorEditor::getFontColor( const QColor& backgroundColor ) const
{
    const int THRESHOLD       = 105;
    int       backgroundDelta = ( backgroundColor.red() * 0.299 ) + ( backgroundColor.green() * 0.587 ) +
                          ( backgroundColor.blue() * 0.114 );
    return QColor( ( 255 - backgroundDelta < THRESHOLD ) ? Qt::black : Qt::white );
}

} // end namespace caffa
