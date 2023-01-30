// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2021- 3D-Radar AS
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
#include "cafDynamicUniqueCast.h"
#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafObjectCapability.h"
#include "cafSignal.h"

#include <list>
#include <memory>
#include <set>
#include <vector>

namespace caffa
{
class FieldCapability;
class ObjectFactory;

/**
 * The base class of all objects
 */
class ObjectHandle : public SignalObserver, public SignalEmitter
{
public:
    using Predicate = std::function<bool( const ObjectHandle* )>;

    ObjectHandle();
    virtual ~ObjectHandle() noexcept;

    static std::string classKeywordStatic(); // For IoFieldCap to be able to handle fields of ObjectHandle directly
    static std::vector<std::string> classInheritanceStackStatic();

    /// The classKeyword method is overridden in subclasses by the CAFFA_HEADER_INIT macro
    virtual std::string              classKeyword() const                                         = 0;
    virtual bool                     matchesClassKeyword( const std::string& classKeyword ) const = 0;
    virtual std::vector<std::string> classInheritanceStack() const                                = 0;
    virtual std::string              classDocumentation() const { return ""; }

    /**
     * The registered fields contained in this Object.
     * @return a vector of FieldHandle pointers
     */
    std::vector<FieldHandle*> fields() const;

    /**
     * Find a particular field by keyword
     * @param keyword
     * @return a FieldHandle pointer
     */
    FieldHandle* findField( const std::string& keyword ) const;

    /**
     * The field referencing this object as a child
     * @return a FieldHandle pointer to the parent. If called on a root object this is nullptr
     */
    FieldHandle* parentField() const;

    /**
     * Traverses all children recursively to find objects matching the predicate.
     * This object is also included if it matches.
     * @param predicate a function pointer predicate
     * @return a list of matching descendants
     */
    std::list<ObjectHandle*> matchingDescendants( Predicate predicate ) const;

    /**
     * Get a list of all child objects
     * @return a list of matching children
     */
    std::list<ObjectHandle*> children() const;

    // Perform cleanup before delete
    void prepareForDelete() noexcept;

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
     * @return std::unique_ptr<Object>
     */
    virtual std::unique_ptr<ObjectHandle> deepClone( caffa::ObjectFactory* optionalObjectFactory = nullptr ) const = 0;

    /// Method gets called from Document after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead(){};
    /// Method gets called from Document before saving document.
    /// Re-implement to make sure your fields have correct data before saving
    virtual void setupBeforeSave(){};

protected:
    /**
     * Add a field to the object
     */
    void addField( FieldHandle* field, const std::string& keyword );

private:
    ObjectHandle( const ObjectHandle& )            = delete;
    ObjectHandle& operator=( const ObjectHandle& ) = delete;

    // Fields
    std::vector<FieldHandle*> m_fields;

    // Capabilities
    std::vector<std::unique_ptr<ObjectCapability>> m_capabilities;

    // Child/Parent Relationships
    void setAsParentField( FieldHandle* parentField );
    void detachFromParentField();
    void disconnectObserverFromAllSignals( SignalObserver* observer );

    FieldHandle* m_parentField;

    // Give access to set/removeAsParentField
    template <class T>
    friend class ChildArrayField;
    friend class ChildFieldDirectStorageAccessor;
    friend class ChildArrayFieldDirectStorageAccessor;

    template <class T>
    friend class Field; // For backwards compatibility layer

    template <class T>
    friend class FieldJsonCap;

    // Support system for Pointer
    friend class ObservingPointerImpl;
    std::set<ObjectHandle**> m_pointersReferencingMe;
};

} // namespace caffa

#include "cafFieldHandle.h"
