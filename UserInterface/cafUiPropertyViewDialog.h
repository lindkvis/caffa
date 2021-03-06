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

#pragma once

#include <QDialog>
#include <QDialogButtonBox>

namespace caffa
{
class Object;
class UiPropertyView;

class UiPropertyViewDialog : public QDialog
{
public:
    UiPropertyViewDialog( QWidget* parent, Object* object, const QString& windowTitle );
    UiPropertyViewDialog( QWidget*                                 parent,
                          Object*                                  object,
                          const QString&                           windowTitle,
                          const QDialogButtonBox::StandardButtons& standardButtons );
    ~UiPropertyViewDialog() override;

    QDialogButtonBox* dialogButtonBox();

private:
    void initialize( Object* object, const QString& windowTitle );
    void setupUi();

private:
    QString           m_windowTitle;
    Object*           m_object;
    UiPropertyView*   m_uiPropertyView;
    QDialogButtonBox* m_buttonBox;
};

} // End of namespace caffa
