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


#include "cafPdmWebFileUploadEditor.h"

#include "cafAssert.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiOrdering.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLink.h>
#include <Wt/WMessageBox.h>
#include <Wt/WProgressBar.h>
#include <Wt/WWidget.h>

#include <QFileInfo>
#include <QDir>


namespace caf
{

CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT(PdmWebFileUploadEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(m_fileUpload);
    CAF_ASSERT(m_label);

    applyTextToLabel(m_label.get(), uiConfigName);

    Wt::WLink   link;
    std::string fullLinkText;
    QString fieldValue = uiField()->uiValue().toString();
    if (!fieldValue.isEmpty())
    {
        QDir dir = uploadDir();
        QDir webroot (QString::fromStdString(wApp->docRoot()));
        QString fullPath = dir.absoluteFilePath(fieldValue);
        fullLinkText = fieldValue.toStdString();
        link = Wt::WLink(Wt::LinkType::Url, "/" + webroot.relativeFilePath(fullPath).toStdString());
    }

    std::string shortLinkText = fullLinkText;
    if (fullLinkText.length() > 20)
    {
        shortLinkText = fullLinkText.substr(0, 8) + "..." + fullLinkText.substr(fullLinkText.length() - 6);
    }

    m_fileLink->setText(shortLinkText);
    m_fileLink->setLink(link);

    m_label->setDisabled(uiField()->isUiReadOnly(uiConfigName));
    m_fileLink->setDisabled(uiField()->isUiReadOnly(uiConfigName) || m_fileLink->link().isNull());
    m_fileLink->setHidden(m_fileLink->link().isNull());
    m_fileLink->setToolTip(fullLinkText);
    m_fileUpload->setDisabled(uiField()->isUiReadOnly(uiConfigName));
    m_fileUpload->setToolTip(uiField()->uiToolTip(uiConfigName).toStdString());
    m_fileUpload->setFilters(m_attributes.m_fileSelectionFilter.toStdString());
    m_fileUpload->setDisplayWidget(m_uploadButton.get());
    m_fileUpload->setProgressBar(std::make_unique<Wt::WProgressBar>());

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebFileUploadEditor::createEditorWidget()
{
    Wt::WContainerWidget* container = new Wt::WContainerWidget();
    container->setContentAlignment(Wt::AlignmentFlag::Top);
    Wt::WHBoxLayout* layout = container->setLayout<Wt::WHBoxLayout>(std::make_unique<Wt::WHBoxLayout>());
    layout->setContentsMargins(0, 0, 0, 0);
    m_fileLink = new Wt::WAnchor;
    m_fileUpload = new Wt::WFileUpload;

    layout->addWidget(std::unique_ptr<Wt::WAnchor>(m_fileLink.get()));
    layout->addWidget(std::unique_ptr<Wt::WFileUpload>(m_fileUpload.get()));
    m_uploadButton = new Wt::WPushButton;
    m_uploadButton->setText("Browse");
    m_uploadButton->setStyleClass("btn-primary");
    layout->addWidget(std::unique_ptr<Wt::WPushButton>(m_uploadButton.get()));

    m_fileUpload->changed().connect([=] { slotFileSelectionFinished(); });
    m_fileUpload->uploaded().connect([=] { slotFileUploaded(); });
    m_fileUpload->fileTooLarge().connect([=] { slotFileTooLargeError(); });

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
    m_fileUpload->stealSpooledFile();
    std::string tmpFilePath = m_fileUpload->spoolFileName();
    QFileInfo tmpFileInfo(QString::fromStdString(tmpFilePath));
    QFileInfo clientFileInfo(QString::fromStdString(m_fileUpload->clientFileName().narrow()));
    QString fileName = clientFileInfo.fileName();
    QDir destinationDir = uploadDir();
    QString destinationFilePath = destinationDir.filePath(destinationDir.absoluteFilePath(fileName));
    QFile file (tmpFileInfo.filePath());
    file.rename(destinationFilePath);

    QVariant v;
    v = fileName;
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebFileUploadEditor::slotFileTooLargeError()
{
    auto messageBox =
        m_fileUpload->addChild(std::make_unique<Wt::WMessageBox>(
            "Error",
            Wt::WString("<p>File uploaded is too large</p>"
                        "<p>The maximum file size allowed is {1} KiB").arg(wApp->maximumRequestSize() / 1024),
            Wt::Icon::Critical,
            Wt::StandardButton::Ok));

    messageBox->setModal(false);

    messageBox->buttonClicked().connect([=] {
        m_fileUpload->removeChild(messageBox);
    });

    messageBox->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDir PdmWebFileUploadEditor::uploadDir()
{
    Wt::WString docRoot = Wt::WApplication::instance()->docRoot();
    QDir uploadDir (QString::fromStdString(docRoot.narrow()));
    bool worked = uploadDir.cd("uploads");
    CAF_ASSERT(worked);
    return uploadDir;
}

} // end namespace caf
