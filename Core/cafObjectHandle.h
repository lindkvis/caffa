//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//   Copyright (C) 2013- Ceetron Solutions AS
//   Copyright (C) 2021- 3D-Radar AS
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

#include "cafAssert.h"
#include "cafBase.h"
#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafSignal.h"

#include <any>
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace caffa
{
class ObjectCapability;
class FieldCapability;

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

    virtual std::string classKeywordDynamic() const = 0;

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
     * @param takeOwnership boolean stating whether the ObjectHandle takes
     *                      over the responsibility of the object.
     */
    void addCapability( ObjectCapability* capability, bool takeOwnership )
    {
        m_capabilities.push_back( std::make_pair( capability, takeOwnership ) );
    }

    /**
     * Get an object capability of the given type
     * @return a typed object capability
     */
    template <typename CapabilityType>
    CapabilityType* capability() const
    {
        for ( auto capabilityAndOwnership : m_capabilities )
        {
            CapabilityType* capability = dynamic_cast<CapabilityType*>( capabilityAndOwnership.first );
            if ( capability ) return capability;
        }
        return nullptr;
    }

    virtual std::string uuid() const { return ""; }
    virtual void        setUuid( const std::string& ) {}

protected:
    /**
     * Add a field to the object
     */
    void addField( FieldHandle* field, const std::string& keyword );

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( ObjectHandle );

    // Fields
    std::vector<FieldHandle*> m_fields;

    // Capabilities
    std::vector<std::pair<ObjectCapability*, bool>> m_capabilities;

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

template <typename ToObjectHandleDerivedClass, typename FromObjectHandleDerivedClass = ObjectHandle>
bool dynamic_unique_cast_is_valid( const std::unique_ptr<FromObjectHandleDerivedClass>& fromPointer )
{
    return dynamic_cast<ToObjectHandleDerivedClass*>( fromPointer.get() ) != nullptr;
}

template <typename ToObjectHandleDerivedClass, typename FromObjectHandleDerivedClass = ObjectHandle>
std::unique_ptr<ToObjectHandleDerivedClass> dynamic_unique_cast( std::unique_ptr<FromObjectHandleDerivedClass> fromPointer )
{
    if ( !dynamic_unique_cast_is_valid<ToObjectHandleDerivedClass, FromObjectHandleDerivedClass>( fromPointer ) )
    {
        CAFFA_ERROR( "Bad cast! " << typeid( FromObjectHandleDerivedClass ).name() << " cannot be cast to "
                                  << typeid( ToObjectHandleDerivedClass ).name() );
        CAFFA_ASSERT( false );
        return nullptr;
    }

    std::unique_ptr<ToObjectHandleDerivedClass> toPointer(
        static_cast<ToObjectHandleDerivedClass*>( fromPointer.release() ) );
    return toPointer;
}

} // namespace caffa

#include "cafFieldHandle.h"
