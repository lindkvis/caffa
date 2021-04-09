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

#include "cafFieldHandle.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafIconProvider.h"
#include "cafInternalUiFieldCapability.h"
#include "cafObjectCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"
#include "cafObjectIoCapability.h"
#include "cafObjectUiCapability.h"
#include "cafPdmPointer.h"
#include "cafUiFieldSpecialization.h"
#include "cafUiOrdering.h"

#include <set>

namespace caf
{
class FieldHandle;
template <class FieldDataType>
class Field;
class UiEditorAttribute;
class PdmUiTreeOrdering;
class ObjectCapability;

#define CAF_HEADER_INIT CAF_IO_HEADER_INIT
#define CAF_SOURCE_INIT CAF_IO_SOURCE_INIT
#define CAF_ABSTRACT_SOURCE_INIT CAF_IO_ABSTRACT_SOURCE_INIT

/// InitField sets the file keyword for the field,
/// adds the field to the internal data structure in the Object,
/// sets the default value for the field,
/// and sets up the static user interface related information for the field
/// Note that classKeyword() is not virtual in the constructor of the Object
/// This is expected and fine.

#define CAF_InitField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis )                                \
    {                                                                                                                         \
        CAF_PDM_VERIFY_IO_KEYWORD( keyword )                                                                                  \
                                                                                                                              \
        static bool checkingThePresenceOfHeaderAndSourceInitMacros =                                                          \
            Error_You_forgot_to_add_the_macro_CAF_IO_HEADER_INIT_and_or_CAF_IO_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        this->isInheritedFromPdmUiObject();                                                                                   \
        this->isInheritedFromSerializable();                                                                                  \
                                                                                                                              \
        AddIoCapabilityToField( field );                                                                                      \
        AddUiCapabilityToField( field );                                                                                      \
        RegisterClassWithField( classKeyword(), field );                                                                      \
                                                                                                                              \
        caf::UiItemInfo objDescr( uiName, std::string( iconResourceName ), toolTip, whatsThis, keyword );                     \
        addFieldUi( field, keyword, default, objDescr );                                                                      \
    }

/// InitFieldNoDefault does the same as InitField but omits the default value.
/// Note that classKeyword() is not virtual in the constructor of the Object
/// This is expected and fine.

#define CAF_InitFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis )                                \
    {                                                                                                                         \
        static bool checkingThePresenceOfHeaderAndSourceInitMacros =                                                          \
            Error_You_forgot_to_add_the_macro_CAF_IO_HEADER_INIT_and_or_CAF_IO_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        this->isInheritedFromPdmUiObject();                                                                                   \
        this->isInheritedFromSerializable();                                                                                  \
                                                                                                                              \
        AddIoCapabilityToField( field );                                                                                      \
        AddUiCapabilityToField( field );                                                                                      \
        RegisterClassWithField( classKeyword(), field );                                                                      \
                                                                                                                              \
        caf::UiItemInfo objDescr( uiName, std::string( iconResourceName ), toolTip, whatsThis, keyword );                     \
        addFieldUiNoDefault( field, keyword, objDescr );                                                                      \
    }

} // End of namespace caf

namespace caf
{
class Object : public ObjectHandle, public ObjectIoCapability, public ObjectUiCapability
{
public:
    CAF_HEADER_INIT;

    Object();
    ~Object() override {}

    /// InitObject sets up the user interface related information for the object
    /// Placed in the constructor of your Object
    /// Note that classKeyword() is not virtual in the constructor of the Object
    /// This is expected and fine.
    Object& initObject();

    Object& initUi( const std::string& uiName           = "",
                    const std::string& iconResourceName = "",
                    const std::string& toolTip          = "",
                    const std::string& whatsThis        = "" );

    template <typename FieldType>
    FieldType& initField( FieldType& field, const std::string& keyword )
    {
        isInheritedFromPdmUiObject();
        isInheritedFromSerializable();
        AddIoCapabilityToField( &field );
        AddUiCapabilityToField( &field );

        RegisterClassWithField( classKeyword(), &field );
        addField( &field, keyword );
        return field;
    }

    /// Adds field to the internal data structure and sets the file keyword and Ui information
    /// Consider this method private. Please use the CAF_InitField() macro instead
    template <typename FieldDataType>
    void addFieldUi( Field<FieldDataType>* field,
                     const std::string&    keyword,
                     const FieldDataType&  defaultValue,
                     const UiItemInfo&     fieldDescription )
    {
        addFieldUiNoDefault( field, keyword, fieldDescription );
        field->setDefaultValue( defaultValue );
        *field = defaultValue;
    }

    /// Does the same as the above method, but omits the default value.
    /// Consider this method private. Please use the CAF_InitFieldNoDefault() macro instead.
    void addFieldUiNoDefault( FieldHandle* field, const std::string& keyword, const UiItemInfo& fieldDescription )
    {
        addField( field, keyword );

        FieldUiCapability* uiFieldHandle = field->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            uiFieldHandle->setUiItemInfo( fieldDescription );
        }
    }

    /// Returns _this_ if _this_ has requested class keyword
    /// Traverses parents recursively and returns first parent of the requested
    /// type.
    void firstAncestorOrThisFromClassKeyword( const std::string& classKeyword, Object*& ancestor ) const;

    /// Traverses all children recursively to find objects of the requested
    /// class keyword. This object is also
    /// included if it has the requested class keyword
    void descendantsIncludingThisFromClassKeyword( const std::string& classKeyword, std::vector<Object*>& descendants ) const;

    /// Gets all children matching class keyword. Not recursive.
    void childrenFromClassKeyword( const std::string& classKeyword, std::vector<Object*>& children ) const;
};

} // End of namespace caf
