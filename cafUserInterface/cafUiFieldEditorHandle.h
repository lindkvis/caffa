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
#pragma once

#include "cafFactory.h"
#include "cafUiEditorAttribute.h"
#include "cafUiEditorHandle.h"

#include <string>
#include <vector>

#include <QLabel>
#include <QPointer>
#include <QString>
#include <QWidget>

class QLabel;

namespace caffa
{
//==================================================================================================
/// Macros helping in development of CAF UI editors
//==================================================================================================

/// CAF_UI_FIELD_EDITOR_HEADER_INIT assists the factory used when creating editors
/// Place this in the header file inside the class definition of your UiEditor

#define CAF_UI_FIELD_EDITOR_HEADER_INIT \
public:                                 \
    static std::string uiEditorTypeName()

/// CAF_UI_FIELD_EDITOR_SOURCE_INIT implements editorTypeName() and registers the field editor in the field editor
/// factory Place this in the cpp file, preferably above the constructor

#define CAF_UI_FIELD_EDITOR_SOURCE_INIT( EditorClassName )                       \
    std::string EditorClassName::uiEditorTypeName() { return #EditorClassName; } \
    CAF_FACTORY_REGISTER( caffa::UiFieldEditorHandle, EditorClassName, std::string, EditorClassName::uiEditorTypeName() )

/// CAF_UI_REGISTER_DEFAULT_FIELD_EDITOR registers what default editor to use with a field of a certain type
/// Place this in the cpp file, preferably above the constructor

#define CAF_UI_REGISTER_DEFAULT_FIELD_EDITOR( EditorClassName, TypeName ) \
    CAF_FACTORY_REGISTER( caffa::UiFieldEditorHandle, EditorClassName, std::string, typeid( caffa::Field<TypeName> ).name() );

class UiGroup;
class FieldUiCapability;

//==================================================================================================
/// Abstract class to handle editors of Fields
//==================================================================================================

class UiFieldEditorHandle : public QObject, public UiEditorHandle
{
    Q_OBJECT
public:
    UiFieldEditorHandle();
    ~UiFieldEditorHandle() override;

    FieldUiCapability* uiField();
    void               setUiField( FieldUiCapability* uiFieldHandle );

    void     createWidgets( QWidget* parent );
    QWidget* combinedWidget() { return m_combinedWidget; }
    QWidget* editorWidget() { return m_editorWidget; }
    QWidget* labelWidget() { return m_labelWidget; }
    QMargins labelContentMargins() const;
    int      rowStretchFactor() const;

protected: // Virtual interface to override
    /// Implement one of these, or both editor and label. The widgets will be used in the parent layout according to
    /// being "Label" Editor" or a single combined widget.

    virtual QWidget* createCombinedWidget( QWidget* parent ) { return nullptr; }
    virtual QWidget* createEditorWidget( QWidget* parent ) { return nullptr; }
    virtual QWidget* createLabelWidget( QWidget* parent ) { return nullptr; }

    void setValueToField( const Variant& value );

    void             updateLabelFromField( QLabel* label ) const;
    virtual QMargins calculateLabelContentMargins() const;
    virtual bool     isMultiRowEditor() const;

private:
    void updateContextMenuPolicy();

private slots:
    void customMenuRequested( QPoint pos );

private:
    QPointer<QWidget> m_combinedWidget;
    QPointer<QWidget> m_editorWidget;
    QPointer<QWidget> m_labelWidget;
};

} // End of namespace caffa
