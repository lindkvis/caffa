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
#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafAssert.h"
#include "cafFactory.h"
#include "cafUiEditorHandle.h"

#include <vector>

#include <Wt/Core/observing_ptr.hpp>

#include <string>

namespace Wt
{
class WContainerWidget;
class WLabel;
class WWidget;
} // namespace Wt

namespace caf
{
//==================================================================================================
/// Macros helping in development of PDM UI editors
//==================================================================================================

/// CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT assists the factory used when creating editors
/// Place this in the header file inside the class definition of your UiEditor

#define CAF_PDM_WEB_FIELD_EDITOR_HEADER_INIT \
public:                                      \
    static std::string uiEditorTypeName()

/// CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT implements editorTypeName() and registers the field editor in the field editor
/// factory Place this in the cpp file, preferably above the constructor

#define CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( EditorClassName )                  \
    std::string EditorClassName::uiEditorTypeName() { return #EditorClassName; } \
    CAF_FACTORY_REGISTER( caf::PdmWebFieldEditorHandle, EditorClassName, std::string, EditorClassName::uiEditorTypeName() )

/// CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR registers what default editor to use with a field of a certain type
/// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_WEB_REGISTER_DEFAULT_FIELD_EDITOR( EditorClassName, TypeName )                                                 \
    CAF_FACTORY_REGISTER( caf::PdmWebFieldEditorHandle, EditorClassName, std::string, typeid( caf::Field<TypeName> ).name() ); \
    CAF_FACTORY_REGISTER2( caf::PdmWebFieldEditorHandle,                                                                       \
                           EditorClassName,                                                                                    \
                           std::string,                                                                                        \
                           typeid( caf::ProxyValueField<TypeName> ).name() );

class UiGroup;
class FieldUiCapability;

//==================================================================================================
/// Abstract class to handle editors of Fields
//==================================================================================================

class PdmWebFieldEditorHandle : public UiEditorHandle
{
public:
    PdmWebFieldEditorHandle();
    ~PdmWebFieldEditorHandle() override;

    FieldUiCapability* uiField();
    void               setUiField( FieldUiCapability* uiFieldHandle );

    virtual bool                 hasLabel() const;
    void                         applyTextToLabel( Wt::WLabel* label ) const;
    std::unique_ptr<Wt::WWidget> findOrCreateCombinedWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets );
    std::unique_ptr<Wt::WWidget> findOrCreateEditorWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets );
    std::unique_ptr<Wt::WLabel>  findOrCreateLabelWidget( std::list<std::unique_ptr<Wt::WWidget>>& existingWidgets );
    int                          rowStretchFactor() const;

protected: // Virtual interface to override
    /// Implement one of these, or both editor and label. The widgets will be used in the parent layout according to
    /// being "Label" Editor" or a single combined widget.

    virtual Wt::WContainerWidget* createCombinedWidget() { return nullptr; }
    virtual Wt::WWidget*          createEditorWidget() { return nullptr; }
    virtual Wt::WLabel*           createLabelWidget() { return nullptr; }

    void         setValueToField( const Variant& value );
    virtual bool isMultiRowEditor() const;

private:
    void updateContextMenuPolicy();

    Wt::Core::observing_ptr<Wt::WWidget> m_combinedWidget;
    Wt::Core::observing_ptr<Wt::WWidget> m_editorWidget;
    Wt::Core::observing_ptr<Wt::WLabel>  m_labelWidget;
};

} // End of namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
