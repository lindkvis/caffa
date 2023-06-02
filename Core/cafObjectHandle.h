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

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafMethodHandle.h"
#include "cafObjectCapability.h"
#include "cafSignal.h"
#include "cafStringTools.h"

#include <array>
#include <list>
#include <memory>
#include <set>
#include <string_view>
#include <type_traits>
#include <vector>

namespace caffa
{
class FieldCapability;
class ObjectFactory;
class Inspector;
class Editor;

/**
 * The base class of all objects
 */
class ObjectHandle : public SignalObserver, public SignalEmitter
{
public:
    using InheritanceStackType = std::vector<std::string>;
    using Ptr                  = std::shared_ptr<ObjectHandle>;
    using ConstPtr             = std::shared_ptr<const ObjectHandle>;

    ObjectHandle();
    virtual ~ObjectHandle() noexcept;

    // TODO: Once compilers support constexpr std::vector and std::string these can be made constexpr
    static std::string  classKeywordStatic() { return "ObjectHandle"; }
    virtual std::string classKeyword() const { return classKeywordStatic(); }

    virtual InheritanceStackType classInheritanceStack() const { return { classKeywordStatic() }; }

    static bool matchesClassKeyword( const std::string& classKeyword, const InheritanceStackType& inheritanceStack )
    {
        return std::any_of( inheritanceStack.begin(),
                            inheritanceStack.end(),
                            [&classKeyword]( const std::string& testKeyword ) { return classKeyword == testKeyword; } );
    }

    static constexpr bool isValidCharacter( char c )
    {
        return caffa::StringTools::isalpha( c ) || caffa::StringTools::isdigit( c ) || c == '_' || c == ':';
    }

    /**
     * Checks whether the keyword is a valid one.
     * We accept regular letters and underscore '_'
     */
    static constexpr bool isValidKeyword( const std::string_view& type )
    {
        if ( caffa::StringTools::isdigit( type[0] ) ) return false;

        auto end = std::find( type.begin(), type.end(), '\0' );

        auto validCount = std::count_if( type.cbegin(), end, ObjectHandle::isValidCharacter );
        auto invalidCount =
            std::count_if( type.cbegin(), end, []( auto c ) { return !ObjectHandle::isValidCharacter( c ); } );

        return validCount > 0u && invalidCount == 0u;
    }

    virtual std::string classDocumentation() const { return ""; }

    /**
     * The registered fields contained in this Object.
     * @return a vector of FieldHandle pointers
     */
    std::list<FieldHandle*> fields() const;

    /**
     * The registered methods for this Object.
     * @return a list of MethodHandle pointers
     */
    std::list<MethodHandle*> methods() const;

    /**
     * Find a particular field by keyword
     * @param keyword
     * @return a FieldHandle pointer
     */
    FieldHandle* findField( const std::string& keyword ) const;

    /**
     * Find a particular method by keyword
     * @param keyword
     * @return a MethodHandle pointer
     */
    MethodHandle* findMethod( const std::string& keyword ) const;

    /**
     * Add an object capability to the object
     * @param capability the new capability
     */
    void addCapability( std::unique_ptr<ObjectCapability> capability )
    {
        m_capabilities.push_back( std::move( capability ) );
    }

    /**
     * Get an object capability of the given type
     * @return a typed object capability
     */
    template <typename CapabilityType>
    CapabilityType* capability() const
    {
        for ( auto& cap : m_capabilities )
        {
            CapabilityType* capability = dynamic_cast<CapabilityType*>( cap.get() );
            if ( capability ) return capability;
        }
        return nullptr;
    }

    virtual std::string uuid() const { return ""; }
    virtual void        setUuid( const std::string& ) {}

    /**
     * @brief Deep clone the object using an optional object factory
     *
     * @param optionalObjectFactory if null the default object factory will be used
     * @return std::shared_ptr<Object>
     */
    virtual ObjectHandle::Ptr deepClone( caffa::ObjectFactory* optionalObjectFactory = nullptr ) const = 0;

    /// Method gets called from Document after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead(){};

    void disconnectObserverFromAllSignals( SignalObserver* observer );

    /**
     * Accept the visit by an inspecting visitor
     * @param visitor
     */
    void accept( Inspector* visitor ) const;

    /**
     * Accept the visit by an editing visitor
     * @param visitor
     */
    void accept( Editor* visitor );

protected:
    /**
     * Add a field to the object
     */
    void addField( FieldHandle* field, const std::string& keyword );

    /**
     * Add a method to the object
     */
    void addMethod( MethodHandle* method, const std::string& keyword, MethodHandle::Type type );

private:
    ObjectHandle( const ObjectHandle& )            = delete;
    ObjectHandle& operator=( const ObjectHandle& ) = delete;

    // Fields
    std::map<std::string, FieldHandle*> m_fields;

    // Methods
    std::map<std::string, MethodHandle*> m_methods;

    // Capabilities
    std::vector<std::unique_ptr<ObjectCapability>> m_capabilities;

    // Support system for Pointer
    friend class ObservingPointerImpl;
    std::set<ObjectHandle**> m_pointersReferencingMe;
};

template <typename T>
concept DerivesFromObjectHandle = std::is_base_of<ObjectHandle, T>::value;

} // namespace caffa
