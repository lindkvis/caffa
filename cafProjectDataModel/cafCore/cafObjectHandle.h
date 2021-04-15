#pragma once

#include "cafAssert.h"
#include "cafBase.h"
#include "cafFieldHandle.h"
#include "cafSignal.h"
#include "cafVariant.h"

#include <any>
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace caf
{
class ObjectCapability;
class ObjectUiCapability;
class FieldIoCapability;
class FieldCapability;
class ObjectXmlCapability;
class ChildArrayFieldHandle;

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
    static std::vector<std::string> classKeywordAliases();

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

    /// Traverses all children recursively to find objects of the requested type. This object is NOT
    /// included if it is of the requested type.
    template <typename T>
    std::list<T*> descendantsOfType() const;

    /**
     * Get a list of all child objects
     * @return a list of matching children
     */
    std::list<ObjectHandle*> children() const;

    // PtrReferences
    /// The PtrField's containing pointers to this Objecthandle
    /// Use ownerObject() on the fieldHandle to get the ObjectHandle
    void referringPtrFields( std::vector<FieldHandle*>& fieldsReferringToMe ) const;
    /// Convenience method to get the objects pointing to this field
    void objectsWithReferringPtrFields( std::vector<ObjectHandle*>& objects ) const;
    /// Convenience method to get the objects of specified type pointing to this field
    template <typename T>
    void objectsWithReferringPtrFieldsOfType( std::vector<T*>& objectsOfType ) const;

    // Detach object from all referring fields
    void prepareForDelete();

    void fieldChangedByCapability( const FieldHandle*     field,
                                   const FieldCapability* changedCapability,
                                   const Variant&         oldValue,
                                   const Variant&         newValue );

    // Object capabilities
    void addCapability( ObjectCapability* capability, bool takeOwnership )

    {
        m_capabilities.push_back( std::make_pair( capability, takeOwnership ) );
    }

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

    /// Field used to toggle object enabled/disabled
    virtual caf::FieldHandle* objectToggleField() { return nullptr; }
    /// Field holding the object description
    virtual caf::FieldHandle* userDescriptionField() { return nullptr; }

protected:
    void addField( FieldHandle* field, const std::string& keyword );

    /// Method to reimplement to catch when the field has changed due to changes in capability
    virtual void onFieldChangedByCapability( const FieldHandle*     field,
                                             const FieldCapability* changedCapability,
                                             const Variant&         oldValue,
                                             const Variant&         newValue )
    {
    }

private:
    CAF_DISABLE_COPY_AND_ASSIGN( ObjectHandle );

    // Fields
    std::vector<FieldHandle*> m_fields;

    // Capabilities
    std::vector<std::pair<ObjectCapability*, bool>> m_capabilities;

    // Child/Parent Relationships
    void setAsParentField( FieldHandle* parentField );
    void removeAsParentField( FieldHandle* parentField );
    void disconnectObserverFromAllSignals( SignalObserver* observer );

    FieldHandle* m_parentField;

    // PtrReferences
    void                        addReferencingPtrField( FieldHandle* fieldReferringToMe );
    void                        removeReferencingPtrField( FieldHandle* fieldReferringToMe );
    std::multiset<FieldHandle*> m_referencingPtrFields;

    // Give access to set/removeAsParentField
    template <class T>
    friend class ChildArrayField;
    template <class T>
    friend class ChildField;
    template <class T>
    friend class PtrArrayField;
    template <class T>
    friend class PtrField;
    template <class T>
    friend class Field; // For backwards compatibility layer

    template <class T>
    friend class FieldIoCap;

    // Support system for Pointer
    friend class PointerImpl;
    std::set<ObjectHandle**> m_pointersReferencingMe;
};

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::objectsWithReferringPtrFieldsOfType( std::vector<T*>& objectsOfType ) const
{
    std::vector<ObjectHandle*> objectsReferencingThis;
    this->objectsWithReferringPtrFields( objectsReferencingThis );

    for ( auto object : objectsReferencingThis )
    {
        if ( dynamic_cast<T*>( object ) )
        {
            objectsOfType.push_back( dynamic_cast<T*>( object ) );
        }
    }
}
} // namespace caf

#include "cafFieldHandle.h"
