#pragma once

#include "cafAssert.h"
#include "cafPdmBase.h"
#include "cafSignal.h"

#include <QString>

#include <set>
#include <vector>

namespace caf
{
class ObjectCapability;
class FieldHandle;
class ObjectUiCapability;
class FieldIoCapability;
class ObjectXmlCapability;
class ChildArrayFieldHandle;

//==================================================================================================
/// The base class of all objects
//==================================================================================================
class ObjectHandle : public SignalObserver, public SignalEmitter
{
public:
    ObjectHandle();
    virtual ~ObjectHandle();

    static QString classKeywordStatic(); // For PdmXmlFieldCap to be able to handle fields of ObjectHandle directly
    static std::vector<QString> classKeywordAliases();

    /// The registered fields contained in this Object.
    void            fields( std::vector<FieldHandle*>& fields ) const;
    FieldHandle* findField( const QString& keyword ) const;

    /// The field referencing this object as a child
    FieldHandle* parentField() const;

    /// Returns _this_ if _this_ is of requested type
    /// Traverses parents recursively and returns first parent of the requested type.
    template <typename T>
    void firstAncestorOrThisOfType( T*& ancestor ) const;

    /// Traverses parents recursively and returns first parent of the requested type.
    /// Does NOT check _this_ object
    template <typename T>
    void firstAncestorOfType( T*& ancestor ) const;

    /// Calls firstAncestorOrThisOfType, and asserts that a valid object is found
    template <typename T>
    void firstAncestorOrThisOfTypeAsserted( T*& ancestor ) const;

    template <typename T>
    void allAncestorsOfType( std::vector<T*>& ancestors ) const;

    template <typename T>
    void allAncestorsOrThisOfType( std::vector<T*>& ancestors ) const;

    /// Traverses all children recursively to find objects of the requested type. This object is also
    /// included if it is of the requested type.
    template <typename T>
    void descendantsIncludingThisOfType( std::vector<T*>& descendants ) const;

    /// Traverses all children recursively to find objects of the requested type. This object is NOT
    /// included if it is of the requested type.
    template <typename T>
    void descendantsOfType( std::vector<T*>& descendants ) const;

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

    // Object capabilities
    void addCapability( ObjectCapability* capability, bool takeOwnership )
    {
        m_capabilities.push_back( std::make_pair( capability, takeOwnership ) );
    }

    template <typename CapabilityType>
    CapabilityType* capability() const
    {
        for ( size_t i = 0; i < m_capabilities.size(); ++i )
        {
            CapabilityType* capability = dynamic_cast<CapabilityType*>( m_capabilities[i].first );
            if ( capability ) return capability;
        }
        return nullptr;
    }

    virtual void setDeletable( bool isDeletable );
    virtual bool isDeletable() const;
    virtual void onChildDeleted( ChildArrayFieldHandle*           childArray,
                                 std::vector<caf::ObjectHandle*>& referringObjects );

protected:
    void addField( FieldHandle* field, const QString& keyword );

private:
    PDM_DISABLE_COPY_AND_ASSIGN( ObjectHandle );

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
    void                           addReferencingPtrField( FieldHandle* fieldReferringToMe );
    void                           removeReferencingPtrField( FieldHandle* fieldReferringToMe );
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

    // Support system for PdmPointer
    friend class PdmPointerImpl;
    std::set<ObjectHandle**> m_pointersReferencingMe;

    bool m_isDeletable;
};
} // namespace caf

#include "cafFieldHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::firstAncestorOrThisOfType( T*& ancestor ) const
{
    ancestor = nullptr;

    // Check if this matches the type

    const T* objectOfTypeConst = dynamic_cast<const T*>( this );
    if ( objectOfTypeConst )
    {
        ancestor = const_cast<T*>( objectOfTypeConst );
        return;
    }

    this->firstAncestorOfType<T>( ancestor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::firstAncestorOfType( T*& ancestor ) const
{
    ancestor = nullptr;

    // Search parents for first type match
    ObjectHandle* parent      = nullptr;
    FieldHandle*  parentField = this->parentField();
    if ( parentField ) parent = parentField->ownerObject();

    if ( parent != nullptr )
    {
        parent->firstAncestorOrThisOfType<T>( ancestor );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::firstAncestorOrThisOfTypeAsserted( T*& ancestor ) const
{
    firstAncestorOrThisOfType( ancestor );

    CAF_ASSERT( ancestor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::allAncestorsOfType( std::vector<T*>& ancestors ) const
{
    T* firstAncestor = nullptr;
    this->firstAncestorOfType( firstAncestor );
    if ( firstAncestor )
    {
        ancestors.push_back( firstAncestor );
        firstAncestor->allAncestorsOfType( ancestors );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::allAncestorsOrThisOfType( std::vector<T*>& ancestors ) const
{
    T* firstAncestorOrThis = nullptr;
    this->firstAncestorOrThisOfType( firstAncestorOrThis );
    if ( firstAncestorOrThis )
    {
        ancestors.push_back( firstAncestorOrThis );
        firstAncestorOrThis->allAncestorsOfType( ancestors );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::descendantsIncludingThisOfType( std::vector<T*>& descendants ) const
{
    const T* objectOfType = dynamic_cast<const T*>( this );
    if ( objectOfType )
    {
        descendants.push_back( const_cast<T*>( objectOfType ) );
    }

    descendantsOfType( descendants );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectHandle::descendantsOfType( std::vector<T*>& descendants ) const
{
    for ( auto f : m_fields )
    {
        std::vector<ObjectHandle*> childObjects;
        f->childObjects( &childObjects );

        for ( auto childObject : childObjects )
        {
            if ( childObject )
            {
                childObject->descendantsIncludingThisOfType( descendants );
            }
        }
    }
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

} // End of namespace caf
