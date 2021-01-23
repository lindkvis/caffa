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

#include "cafPdmWebDefaultObjectEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4275 4564 )
#endif

#include "cafCoreColor.h"
#include "cafField.h"
#include "cafPdmWebCheckBoxEditor.h"
#include "cafPdmWebCheckBoxTristateEditor.h"
#include "cafPdmWebColorEditor.h"
#include "cafPdmWebDateEditor.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafPdmWebFileUploadEditor.h"
#include "cafPdmWebLineEditor.h"
#include "cafProxyValueField.h"
#include "cafWebValueFieldSpecializations.h"

#include "cafTristate.h"

#include "Wt/WContainerWidget.h"
#include "Wt/WGridLayout.h"
#include "Wt/WPanel.h"

#include <filesystem>

namespace caf
{
// Register default field editor for selected types
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebCheckBoxEditor, bool );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebCheckBoxTristateEditor, caf::Tristate );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebLineEditor, std::string );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebLineEditor, int );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebLineEditor, double );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebLineEditor, float );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebColorEditor, caf::Color );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebDateEditor, Wt::WDate );
CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( PdmWebFileUploadEditor, std::filesystem::path );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebDefaultObjectEditor::PdmWebDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebDefaultObjectEditor::~PdmWebDefaultObjectEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WContainerWidget* PdmWebDefaultObjectEditor::createWidget()
{
    auto widget = new Wt::WContainerWidget;
    widget->setMargin( 0 );
    widget->setContentAlignment( Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Center );
    widget->setObjectName( "ObjectEditor" );
    widget->setMinimumSize( Wt::WLength( 30, Wt::LengthUnit::FontEx ), 150 );
    // widget->setOverflow(Wt::Overflow::Auto, Wt::Orientation::Horizontal);
    widget->setOverflow( Wt::Overflow::Auto, Wt::Orientation::Vertical );

    resetWidget( widget );

    return widget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDefaultObjectEditor::recursivelyConfigureAndUpdateTopLevelUiOrdering( const UiOrdering& topLevelUiOrdering )
{
    Wt::WPanel* panel = dynamic_cast<Wt::WPanel*>( this->widget()->widget( 0 ) );
    CAF_ASSERT( panel );
    Wt::WContainerWidget* panelContent = dynamic_cast<Wt::WContainerWidget*>( panel->centralWidget() );
    panelContent->setContentAlignment( Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Left );
    CAF_ASSERT( panelContent );
    Wt::WGridLayout* gridLayout = dynamic_cast<Wt::WGridLayout*>( panelContent->layout() );
    CAF_ASSERT( gridLayout );
    recursivelyConfigureAndUpdateUiOrderingInGridLayout( topLevelUiOrdering, gridLayout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDefaultObjectEditor::resetWidget( Wt::WContainerWidget* widget )
{
    if ( widget )
    {
        widget->clear();
        Wt::WPanel* panel = widget->addWidget<Wt::WPanel>( std::make_unique<Wt::WPanel>() );
        panel->setMargin( 0 );
        panel->setTitle( "Property Editor" );
        Wt::WContainerWidget* panelContent =
            panel->setCentralWidget<Wt::WContainerWidget>( std::make_unique<Wt::WContainerWidget>() );
        panelContent->setPadding( 0 );
        panelContent->setContentAlignment( Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Left );
        auto gridLayout = std::make_unique<Wt::WGridLayout>();
        gridLayout->setContentsMargins( 0, 0, 0, 0 );
        panelContent->setLayout( std::move( gridLayout ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDefaultObjectEditor::cleanupBeforeSettingObject()
{
    resetWidget( this->widget() );
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
