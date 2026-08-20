// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2021-2022 3D-Radar AS
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################

#pragma once

#include "cafFieldHandle.h"
#include "cafMethodHandle.h"
#include "cafStringTools.h"

#include <algorithm>
#include <map>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

namespace caffa
{
class FieldCapability;
class Inspector;
class Editor;

/**
 * The base class of all objects
 */
class ObjectHandle : public std::enable_shared_from_this<ObjectHandle>
{
public:
    explicit ObjectHandle( bool generateUuid = true );
    virtual ~ObjectHandle() noexcept;

    // TODO: Once compilers support constexpr std::vector and std::string these can be made constexpr
    static constexpr std::string_view                classKeywordStatic() { return "ObjectHandle"; }
    [[nodiscard]] virtual constexpr std::string_view classKeyword() const { return classKeywordStatic(); }

    [[nodiscard]] virtual std::vector<std::string> classInheritanceStack() const
    {
        return { std::string( classKeywordStatic() ) };
    }
    /**
     * @brief Get the parent class keyword
     *
     * @return std::string Class keyword of the parent class
     */
    [[nodiscard]] virtual constexpr std::string_view parentClassKeyword() const { return ""; }

    static bool matchesClassKeyword( const std::string_view&         classKeyword,
                                     const std::vector<std::string>& inheritanceStack );

    static constexpr bool isValidCharacter( const char c )
    {
        return StringTools::isalpha( c ) || StringTools::isdigit( c ) || c == '_' || c == ':';
    }

    /**
     * Checks whether the keyword is a valid one.
     * We accept regular letters and underscore '_'
     */
    template <StringTools::FixedString type>
    static constexpr bool isValidKeyword()
    {
        constexpr std::string_view view = type;
        if ( view == "keyword" ) return false;
        if ( view == "uuid" ) return false;

        if ( StringTools::isdigit( view[0] ) ) return false;

        constexpr auto end = std::ranges::find( view, '\0' );

        constexpr auto validCount = std::count_if( view.cbegin(), end, ObjectHandle::isValidCharacter );
        constexpr auto invalidCount =
            std::count_if( view.cbegin(), end, []( auto c ) { return !ObjectHandle::isValidCharacter( c ); } );

        return validCount > 0u && invalidCount == 0u;
    }

    static bool isValidKeyword( const std::string& type );

    [[nodiscard]] virtual std::string classDocumentation() const { return ""; }

    /**
     * The registered fields contained in this Object.
     * @return a vector of FieldHandle pointers
     */
    [[nodiscard]] std::vector<FieldHandle*> fields() const;

    /**
     * The registered methods for this Object.
     * @return a list of MethodHandle pointers
     */
    [[nodiscard]] std::vector<MethodHandle*> methods() const;

    /**
     * Find a particular field by keyword
     * @param keyword
     * @return a FieldHandle pointer
     */
    [[nodiscard]] FieldHandle* findField( const std::string& keyword ) const;

    /**
     * Find a particular method by keyword
     * @param keyword
     * @return a MethodHandle pointer
     */
    [[nodiscard]] MethodHandle* findMethod( const std::string& keyword ) const;

    [[nodiscard]] const std::string& uuid() const;
    void                             setUuid( const std::string& );

    /// Method gets called from Document after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead() {};

    /**
     * Accept the visit by an inspecting visitor
     * @param visitor
     */
    void accept( Inspector* visitor ) const;

    /**
     * Accept the visit by an editing visitor
     * @param editor
     */
    void accept( Editor* editor );

    ObjectHandle( const ObjectHandle& )            = delete;
    ObjectHandle& operator=( const ObjectHandle& ) = delete;

protected:
    /**
     * Add a field to the object
     */
    void addField( FieldHandle* field, const std::string& keyword );

    /**
     * Add a method to the object
     */
    void addMethod( MethodHandle* method, const std::string& keyword );

private:
    std::string m_uuid;

    // Fields
    std::map<std::string, FieldHandle*> m_fields;

    // Methods
    std::map<std::string, MethodHandle*> m_methods;
};

template <typename T>
concept DerivesFromObjectHandle = std::is_base_of_v<ObjectHandle, T>;

template <typename T>
struct is_shared_ptr : std::false_type
{
};
template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type
{
};

template <typename T>
concept IsSharedPtr = is_shared_ptr<T>::value;

} // namespace caffa
