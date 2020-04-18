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


#include "cafPdmWebColorEditor.h"

#include "cafAssert.h"
#include "cafPdmWebDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <Wt/WAny.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFitLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/Chart/WStandardPalette.h>

#include <memory>

namespace caf
{

CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT(PdmWebColorEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmWebColorEditor::PdmWebColorEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::applyColor(const Wt::WColor& newColor)
{
    if (newColor != m_color)
    {
//        setColorOnWidget(newColor);
        QColor qColor(newColor.red(), newColor.green(), newColor.blue());
        QVariant v;
        v = qColor;
        this->setValueToField(v);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(m_label);

    applyTextToLabel(m_label.get(), uiConfigName);

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }

    QColor col = uiField()->uiValue().value<QColor>();
    setColorOnWidget(Wt::WColor(col.red(), col.green(), col.blue()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebColorEditor::createEditorWidget()
{    
    m_colorSelectionButton = new Wt::WPushButton;
    m_colorSelectionButton->setObjectName("ColorSelectionButton");
    m_colorSelectionButton->setText("...");
    m_colorSelectionButton->clicked().connect(std::bind(&PdmWebColorEditor::colorSelectionClicked, this));

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
    auto dialog = m_colorSelectionButton->addChild(std::make_unique<Wt::WDialog>("Select Color"));
    dialog->positionAt(m_colorSelectionButton.get(), Wt::Orientation::Vertical);
    auto colorGrid = std::make_unique<Wt::WGridLayout>();
    std::vector<Wt::Chart::PaletteFlavour> flavors = { Wt::Chart::PaletteFlavour::Neutral,
                                                       Wt::Chart::PaletteFlavour::Bold,
                                                       Wt::Chart::PaletteFlavour::Muted,
                                                       Wt::Chart::PaletteFlavour::GrayScale
    };
    for (size_t i = 0; i < flavors.size(); ++i)
    {
        Wt::Chart::WStandardPalette palette(flavors[i]);
        for (int j = 0; j < 8; ++j)
        {
            Wt::WColor color = palette.color(j);
            Wt::WColor fontColor = palette.fontColor(j);
            Wt::WCssDecorationStyle style;
            style.setBackgroundColor(color);
            style.setForegroundColor(fontColor);
            style.setBorder(Wt::WBorder(Wt::BorderStyle::Solid, Wt::BorderWidth::Thin, Wt::StandardColor::Black));

            auto pushButton = new Wt::WPushButton;
            pushButton->setDecorationStyle(style);
            pushButton->setThemeStyleEnabled(false);
            pushButton->resize(32, 32);
            pushButton->clicked().connect([=]
                {
                    Wt::WColor color = pushButton->decorationStyle().backgroundColor();
                    dialog->accept();
                    this->applyColor(color);
                    
                }
            );
            colorGrid->addWidget(std::unique_ptr<Wt::WPushButton>(pushButton), i, j);            
        }
    }

    Wt::WPushButton *cancel =
        dialog->footer()->addNew<Wt::WPushButton>("Cancel");
    dialog->rejectWhenEscapePressed();
    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->contents()->setLayout(std::move(colorGrid));
    dialog->show();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebColorEditor::setColorOnWidget(const Wt::WColor& color)
{
    if (m_color != color)
    {
        m_color = color;
    }

    Wt::WColor fontColor      = getFontColor(m_color);
    Wt::WCssDecorationStyle style;
    style.setBackgroundColor(m_color);
    style.setForegroundColor(fontColor);
    m_colorSelectionButton->setDecorationStyle(style);
    m_colorSelectionButton->setThemeStyleEnabled(false);
    
    if (m_attributes.showLabel)
    {
        Wt::WString colorString = color.cssText(m_attributes.showAlpha);
        m_colorSelectionButton->setText(colorString);
    }
}


//--------------------------------------------------------------------------------------------------
/// Based on http://www.codeproject.com/cs/media/IdealTextColor.asp
//--------------------------------------------------------------------------------------------------
Wt::WColor PdmWebColorEditor::getFontColor(const Wt::WColor& backgroundColor) const
{
    const int THRESHOLD = 105;
    int backgroundDelta = (backgroundColor.red() * 0.299) + (backgroundColor.green() * 0.587) + (backgroundColor.blue() * 0.114);
    return Wt::WColor((255 - backgroundDelta < THRESHOLD) ? Wt::StandardColor::Black : Wt::StandardColor::White);
}

} // end namespace caf
