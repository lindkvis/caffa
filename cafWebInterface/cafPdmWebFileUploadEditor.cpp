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
#include "cafPdmWebFileUploadEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafAssert.h"

#include "cafField.h"
#include "cafFieldUiCapability.h"
#include "cafObject.h"
#include "cafUiOrdering.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLink.h>
#include <Wt/WMessageBox.h>
#include <Wt/WProgressBar.h>
#include <Wt/WWidget.h>

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebFileUploadEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::configureAndUpdateUi()
{
    CAF_ASSERT( m_fileUpload );
    CAF_ASSERT( m_label );

    applyTextToLabel( m_label.get() );

    Wt::WLink   link;
    std::string fullLinkText;
    std::string fieldValue = uiField()->uiValue().value<std::string>();
    if ( !fieldValue.empty() )
    {
        std::filesystem::path dir     = uploadDir();
        std::filesystem::path webroot = wApp->docRoot();

        auto fullPath = std::filesystem::absolute( dir / fieldValue );
        fullLinkText  = fieldValue;
        link          = Wt::WLink( Wt::LinkType::Url, "/" + std::filesystem::relative( fullPath, webroot ).string() );
    }

    std::string shortLinkText = fullLinkText;
    if ( fullLinkText.length() > 20 )
    {
        shortLinkText = fullLinkText.substr( 0, 8 ) + "..." + fullLinkText.substr( fullLinkText.length() - 6 );
    }

    m_fileLink->setText( shortLinkText );
    m_fileLink->setLink( link );

    m_label->setDisabled( uiField()->isUiReadOnly() );
    m_fileLink->setDisabled( uiField()->isUiReadOnly() || m_fileLink->link().isNull() );
    m_fileLink->setHidden( m_fileLink->link().isNull() );
    m_fileLink->setToolTip( fullLinkText );
    m_fileUpload->setDisabled( uiField()->isUiReadOnly() );
    m_fileUpload->setToolTip( uiField()->uiToolTip() );
    m_fileUpload->setFilters( m_attributes.m_fileSelectionFilter );
    m_fileUpload->setDisplayWidget( m_uploadButton.get() );
    m_fileUpload->setProgressBar( std::make_unique<Wt::WProgressBar>() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebFileUploadEditor::createEditorWidget()
{
    Wt::WContainerWidget* container = new Wt::WContainerWidget();
    container->setContentAlignment( Wt::AlignmentFlag::Top );
    Wt::WHBoxLayout* layout = container->setLayout<Wt::WHBoxLayout>( std::make_unique<Wt::WHBoxLayout>() );
    layout->setContentsMargins( 0, 0, 0, 0 );
    auto fileLink = std::make_unique<Wt::WAnchor>();
    m_fileLink    = fileLink.get();
    layout->addWidget( std::move( fileLink ) );
    if ( !uploadDir().empty() && std::filesystem::exists( uploadDir() ) )
    {
        auto fileUpload = std::make_unique<Wt::WFileUpload>();
        m_fileUpload    = fileUpload.get();
        layout->addWidget( std::move( fileUpload ) );
        auto uploadButton = std::make_unique<Wt::WPushButton>();
        uploadButton->setText( "Browse" );
        uploadButton->setStyleClass( "btn-primary" );
        m_uploadButton = uploadButton.get();
        layout->addWidget( std::move( uploadButton ) );
        m_fileUpload->changed().connect( [=] { slotFileSelectionFinished(); } );
        m_fileUpload->uploaded().connect( [=] { slotFileUploaded(); } );
        m_fileUpload->fileTooLarge().connect( [=] { slotFileTooLargeError(); } );
    }
    else
    {
        layout->addWidget( std::make_unique<Wt::WLabel>( "Error" ) );
    }

    return container;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebFileUploadEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::slotFileSelectionFinished()
{
    m_fileUpload->upload();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::slotFileUploaded()
{
    auto destinationDir = uploadDir();
    if ( destinationDir.empty() ) return;

    m_fileUpload->stealSpooledFile();
    std::string tmpFilePath = m_fileUpload->spoolFileName();

    std::filesystem::path tmpFileInfo    = tmpFilePath;
    std::filesystem::path clientFileInfo = m_fileUpload->clientFileName().narrow();

    auto fileName            = clientFileInfo.filename();
    auto destinationFilePath = destinationDir / fileName;
    std::filesystem::rename( tmpFileInfo, destinationFilePath );

    Variant v( fileName );
    this->setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::slotFileTooLargeError()
{
    auto messageBox = m_fileUpload->addChild(
        std::make_unique<Wt::WMessageBox>( "Error",
                                           Wt::WString( "<p>File uploaded is too large</p>"
                                                        "<p>The maximum file size allowed is {1} KiB" )
                                               .arg( wApp->maximumRequestSize() / 1024 ),
                                           Wt::Icon::Critical,
                                           Wt::StandardButton::Ok ) );

    messageBox->setModal( false );

    messageBox->buttonClicked().connect( [=] { m_fileUpload->removeChild( messageBox ); } );

    messageBox->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::filesystem::path PdmWebFileUploadEditor::uploadDir()
{
    Wt::WString           docRoot   = Wt::WApplication::instance()->docRoot();
    std::filesystem::path uploadDir = std::filesystem::path( docRoot.narrow() ) / "uploads";
    if ( std::filesystem::exists( uploadDir ) ) return uploadDir;
    return "";
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
