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
#include "cafVariant.h"

#include <any>
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace caffa
{
class ObjectCapability;
class ObjectUiCapability;
class FieldIoCapability;
class FieldCapability;
class ObjectXmlCapability;

/**
 * The base class of all objects
 */
class ObjectHandle : public SignalObserver, public SignalEmitter
{
public:
    /**
     * Signal emitted when the field is changed
     * @param Variant old value
     * @param Variant new value
     */
    Signal<std::tuple<const FieldCapability*, Variant, Variant>> fieldChanged;

public:
    using Predicate = std::function<bool( const ObjectHandle* )>;

    ObjectHandle();
    virtual ~ObjectHandle();

    static std::string classKeywordStatic(); // For IoFieldCap to be able to handle fields of ObjectHandle directly
    static std::vector<std::string> classInheritanceStackStatic();

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
     * Get a list of all ancestors.
     * @return a list of all ancestors
     */
    std::list<ObjectHandle*> ancestors() const;

    /**
     * Get a list of all ancestors matching a predicate
     * @param predicate a function pointer predicate
     * @return a list of all ancestors
     */
    std::list<ObjectHandle*> matchingAncestors( Predicate predicate ) const;

    /**
     * Get a list of all ancestors matching a predicate
     * @param predicate a function pointer predicate
     * @return the first matching ancestor
     */
    ObjectHandle* firstMatchingAncestor( Predicate predicate ) const;

    /**
     * Traverses all children recursively to find objects matching the predicate.
     * This object is also included if it matches.
     * @param predicate a function pointer predicate
     * @return a list of matching descendants
     */
    std::list<ObjectHandle*> matchingDescendants( Predicate predicate ) const;

    /**
     * Traverses all children recursively to find objects of the requested type. This object is NOT
     * included if it is of the requested type.
     * @return list of descendants of a given type
     */
    template <typename T>
    std::list<T*> descendantsOfType() const;

    /**
     * Get a list of all child objects
     * @return a list of matching children
     */
    std::list<ObjectHandle*> children() const;

    // Perform cleanup before delete
    void prepareForDelete();

    void fieldChangedByCapability( const FieldHandle*     field,
                                   const FieldCapability* changedCapability,
                                   const Variant&         oldValue,
                                   const Variant&         newValue );

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

    /**
     * Get a field used to toggle object enabled/disabled
     * @return a field handle to the toggle field
     */
    virtual caffa::FieldHandle* objectToggleField() { return nullptr; }
    /**
     * Field holding the object description, used to provide a user visible label
     * @return a field handle to the user description field
     */
    virtual caffa::FieldHandle* userDescriptionField() { return nullptr; }

protected:
    /**
     * Add a field to the object
     */
    void addField( FieldHandle* field, const std::string& keyword );

    /**
     * Virtual method to reimplement to catch when the field has changed due to changes in capability
     */
    virtual void onFieldChangedByCapability( const FieldHandle*     field,
                                             const FieldCapability* changedCapability,
                                             const Variant&         oldValue,
                                             const Variant&         newValue )
    {
    }

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( ObjectHandle );

    // Fields
    std::vector<FieldHandle*> m_fields;

    // Capabilities
    std::vector<std::pair<ObjectCapability*, bool>> m_capabilities;

    // Child/Parent Relationships
    void setAsParentField( FieldHandle* parentField );
    void removeAsParentField( FieldHandle* parentField );
    void disconnectObserverFromAllSignals( SignalObserver* observer );

    FieldHandle* m_parentField;

    // Give access to set/removeAsParentField
    template <class T>
    friend class ChildArrayField;
    template <class T>
    friend class ChildField;
    template <class T>
    friend class Field; // For backwards compatibility layer

    template <class T>
    friend class FieldIoCap;

    // Support system for Pointer
    friend class PointerImpl;
    std::set<ObjectHandle**> m_pointersReferencingMe;
};

template <typename ToObjectHandleDerivedClass, typename FromObjectHandleDerivedClass = ObjectHandle>
bool static_unique_cast_is_valid( const std::unique_ptr<FromObjectHandleDerivedClass>& fromPointer )
{
    return dynamic_cast<ToObjectHandleDerivedClass*>( fromPointer.get() ) != nullptr;
}

template <typename ToObjectHandleDerivedClass, typename FromObjectHandleDerivedClass = ObjectHandle>
std::unique_ptr<ToObjectHandleDerivedClass> static_unique_cast( std::unique_ptr<FromObjectHandleDerivedClass> fromPointer )
{
    if ( !static_unique_cast_is_valid<ToObjectHandleDerivedClass, FromObjectHandleDerivedClass>( fromPointer ) )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::list<T*> ObjectHandle::descendantsOfType() const
{
    std::list<T*> descendants;
    for ( auto f : m_fields )
    {
        std::vector<ObjectHandle*> childObjects;
        f->childObjects( &childObjects );

        for ( auto childObject : childObjects )
        {
            if ( childObject )
            {
                if ( T* typedObject = dynamic_cast<T*>( childObject ); typedObject )
                {
                    descendants.push_back( typedObject );
                }
                std::list<T*> childDescendants = childObject->descendantsOfType<T>();
                descendants.insert( descendants.end(), childDescendants.begin(), childDescendants.end() );
            }
        }
    }
    return descendants;
}

} // namespace caffa

#include "cafFieldHandle.h"
