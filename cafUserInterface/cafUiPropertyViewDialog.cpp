//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015- Ceetron Solutions AS
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

#include "cafUiPropertyViewDialog.h"

#include "cafObject.h"
#include "cafUiPropertyView.h"

#include <QVBoxLayout>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPropertyViewDialog::UiPropertyViewDialog( QWidget* parent, Object* object, const QString& windowTitle )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    initialize( object, windowTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPropertyViewDialog::UiPropertyViewDialog( QWidget*                                 parent,
                                            Object*                                  object,
                                            const QString&                           windowTitle,
                                            const QDialogButtonBox::StandardButtons& standardButtons )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    m_buttonBox = new QDialogButtonBox( standardButtons );

    initialize( object, windowTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiPropertyViewDialog::~UiPropertyViewDialog()
{
    m_uiPropertyView->showProperties( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* UiPropertyViewDialog::dialogButtonBox()
{
    return m_buttonBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiPropertyViewDialog::initialize( Object* object, const QString& windowTitle )
{
    m_object      = object;
    m_windowTitle = windowTitle;

    setWindowModality( Qt::WindowModal );

    setupUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiPropertyViewDialog::setupUi()
{
    setWindowTitle( m_windowTitle );

    m_uiPropertyView = new UiPropertyView( this );

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( m_uiPropertyView );
    m_uiPropertyView->showProperties( m_object );

    // Buttons
    // CAF_ASSERT(m_buttonBox->buttons().size() > 0);

    connect( m_buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_buttonBox );
}

} // End of namespace caf
