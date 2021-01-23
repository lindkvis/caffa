//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) Ceetron AS
//   Copyright (C) Gaute Lindkvist
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
#include "cafPdmWebColorEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafAssert.h"
#include "cafColor.h"
#include "cafColorTables.h"
#include "cafColorTools.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafFactory.h"

#include <Wt/Chart/WStandardPalette.h>
#include <Wt/WAny.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WFitLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLineEdit.h>

#include <memory>

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebColorEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebColorEditor::PdmWebColorEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::applyColor( const Wt::WColor& newColor )
{
    if ( newColor != m_color )
    {
        //        setColorOnWidget(newColor);
        Color   qColor( newColor.red(), newColor.green(), newColor.blue() );
        Variant v;
        v = qColor;
        this->setValueToField( v );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::configureAndUpdateUi()
{
    CAF_ASSERT( m_label );

    applyTextToLabel( m_label.get() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    Color      col = uiField()->uiValue().value<Color>();
    Wt::WColor setColorOnWidget( col.to<Wt::WColor>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebColorEditor::createEditorWidget()
{
    m_colorSelectionButton = new Wt::WPushButton;
    m_colorSelectionButton->setObjectName( "ColorSelectionButton" );
    m_colorSelectionButton->setText( "..." );
    m_colorSelectionButton->clicked().connect( std::bind( &PdmWebColorEditor::colorSelectionClicked, this ) );

    return m_colorSelectionButton.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebColorEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::colorSelectionClicked()
{
    auto dialog = m_colorSelectionButton->addChild( std::make_unique<Wt::WDialog>( "Select Color" ) );
    dialog->positionAt( m_colorSelectionButton.get(), Wt::Orientation::Vertical );
    auto colorGrid = std::make_unique<Wt::WGridLayout>();

    size_t columns = 4;

    ColorTable colorTable = ColorTables::kellyColors();

    for ( size_t i = 0; i < colorTable.size(); ++i )
    {
        Color      color      = colorTable.cycledColor( i );
        Color      fontColor  = ColorTools::blackOrWhiteContrastColor( color );
        Wt::WColor wColor     = color.to<Wt::WColor>();
        Wt::WColor wFontColor = fontColor.to<Wt::WColor>();

        Wt::WCssDecorationStyle style;
        style.setBackgroundColor( wColor );
        style.setForegroundColor( wFontColor );
        style.setBorder( Wt::WBorder( Wt::BorderStyle::Solid, Wt::BorderWidth::Thin, Wt::StandardColor::Black ) );

        auto pushButton = new Wt::WPushButton;
        pushButton->setDecorationStyle( style );
        pushButton->setThemeStyleEnabled( false );
        pushButton->resize( 32, 32 );
        pushButton->clicked().connect( [=] {
            Wt::WColor color = pushButton->decorationStyle().backgroundColor();
            dialog->accept();
            this->applyColor( color );
        } );
        colorGrid->addWidget( std::unique_ptr<Wt::WPushButton>( pushButton ), i / columns, i % columns );
    }

    Wt::WPushButton* cancel = dialog->footer()->addNew<Wt::WPushButton>( "Cancel" );
    dialog->rejectWhenEscapePressed();
    cancel->clicked().connect( dialog, &Wt::WDialog::reject );

    dialog->contents()->setLayout( std::move( colorGrid ) );
    dialog->show();
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::setColorOnWidget( const Wt::WColor& color )
{
    if ( m_color != color )
    {
        m_color = color;
    }

    Color      cafColor( color.red(), color.green(), color.blue() );
    Color      cafFontColor = ColorTools::blackOrWhiteContrastColor( cafColor );
    Wt::WColor wFontColor   = cafFontColor.to<Wt::WColor>();

    Wt::WCssDecorationStyle style;
    style.setBackgroundColor( m_color );
    style.setForegroundColor( wFontColor );
    m_colorSelectionButton->setDecorationStyle( style );
    m_colorSelectionButton->setThemeStyleEnabled( false );

    if ( m_attributes.showLabel )
    {
        Wt::WString colorString = color.cssText( m_attributes.showAlpha );
        m_colorSelectionButton->setText( colorString );
    }
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
