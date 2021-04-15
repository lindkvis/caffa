
#include "cafObjectHandle.h"

#include "cafAssert.h"
#include "cafChildArrayField.h"
#include "cafFieldHandle.h"
#include "cafObjectCapability.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::ObjectHandle()
    : fieldChanged( this )
{
    m_parentField = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::~ObjectHandle()
{
    this->prepareForDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectHandle::classKeywordStatic()
{
    return classKeywordAliases().front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> ObjectHandle::classKeywordAliases()
{
    return { std::string( "ObjectHandle" ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::FieldHandle*> ObjectHandle::fields() const
{
    return m_fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::setAsParentField( FieldHandle* parentField )
{
    CAF_ASSERT( m_parentField == nullptr );

    m_parentField = parentField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::removeAsParentField( FieldHandle* parentField )
{
    CAF_ASSERT( m_parentField == parentField );

    if ( parentField ) disconnectObserverFromAllSignals( parentField->ownerObject() );

    m_parentField = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::disconnectObserverFromAllSignals( SignalObserver* observer )
{
    if ( observer )
    {
        for ( auto emittedSignal : emittedSignals() )
        {
            emittedSignal->disconnect( observer );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::addReferencingPtrField( FieldHandle* fieldReferringToMe )
{
    if ( fieldReferringToMe != nullptr ) m_referencingPtrFields.insert( fieldReferringToMe );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::removeReferencingPtrField( FieldHandle* fieldReferringToMe )
{
    if ( fieldReferringToMe != nullptr )
    {
        disconnectObserverFromAllSignals( fieldReferringToMe->ownerObject() );
        m_referencingPtrFields.erase( fieldReferringToMe );
    }
}

//--------------------------------------------------------------------------------------------------
/// Appends pointers to all the PtrFields containing a pointer to this object.
/// As the PtrArrayFields can hold several pointers to the same object, the returned vector can
/// contain multiple pointers to the same field.
//--------------------------------------------------------------------------------------------------
void ObjectHandle::referringPtrFields( std::vector<FieldHandle*>& fieldsReferringToMe ) const
{
    std::multiset<FieldHandle*>::const_iterator it;

    for ( it = m_referencingPtrFields.begin(); it != m_referencingPtrFields.end(); ++it )
    {
        fieldsReferringToMe.push_back( *it );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::objectsWithReferringPtrFields( std::vector<ObjectHandle*>& objects ) const
{
    std::vector<caf::FieldHandle*> parentFields;
    this->referringPtrFields( parentFields );
    size_t i;
    for ( i = 0; i < parentFields.size(); i++ )
    {
        objects.push_back( parentFields[i]->ownerObject() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::prepareForDelete()
{
    m_parentField = nullptr;

    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        if ( m_capabilities[i].second ) delete m_capabilities[i].first;
    }

    // Set all guarded pointers pointing to this to NULL
    std::set<ObjectHandle**>::iterator it;
    for ( it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end(); ++it )
    {
        ( **it ) = nullptr;
    }

    m_capabilities.clear();
    m_referencingPtrFields.clear();
    m_pointersReferencingMe.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::fieldChangedByCapability( const FieldHandle*     field,
                                             const FieldCapability* changedCapability,
                                             const Variant&         oldValue,
                                             const Variant&         newValue )
{
    fieldChanged.send( { changedCapability, oldValue, newValue } );

    onFieldChangedByCapability( field, changedCapability, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::addField( FieldHandle* field, const std::string& keyword )
{
    field->m_ownerObject = this;

    CAF_ASSERT( !keyword.empty() );
    CAF_ASSERT( this->findField( keyword ) == nullptr && "Object already has a field with this keyword!" );

    field->setKeyword( keyword );
    m_fields.push_back( field );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ObjectHandle::findField( const std::string& keyword ) const
{
    for ( auto field : fields() )
    {
        if ( field->matchesKeyword( keyword ) )
        {
            return field;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ObjectHandle::parentField() const
{
    return m_parentField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<ObjectHandle*> ObjectHandle::ancestors() const
{
    std::list<ObjectHandle*> allAncestors;

    // Search parents for first type match
    FieldHandle* parentField = this->parentField();
    if ( parentField )
    {
        ObjectHandle* parent = parentField->ownerObject();
        if ( parent )
        {
            allAncestors = parent->ancestors();
            allAncestors.push_back( parent );
        }
    }
    return allAncestors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<ObjectHandle*> ObjectHandle::matchingAncestors( ObjectHandle::Predicate predicate ) const
{
    std::list<ObjectHandle*> ancestors = this->ancestors();

    std::list<ObjectHandle*> matching;
    std::copy_if( ancestors.begin(), ancestors.end(), std::back_inserter( matching ), predicate );
    return matching;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ObjectHandle::firstMatchingAncestor( ObjectHandle::Predicate predicate ) const
{
    // Search parents for first type match
    FieldHandle* parentField = this->parentField();
    if ( parentField )
    {
        ObjectHandle* parent = parentField->ownerObject();
        if ( parent )
        {
            if ( predicate( parent ) ) return parent;
            return parent->firstMatchingAncestor( predicate );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<ObjectHandle*> ObjectHandle::matchingDescendants( ObjectHandle::Predicate predicate ) const
{
    std::list<ObjectHandle*> descendants;
    for ( auto field : m_fields )
    {
        std::vector<ObjectHandle*> childObjects;
        field->childObjects( &childObjects );
        for ( auto childObject : childObjects )
        {
            if ( childObject )
            {
                if ( predicate( childObject ) )
                {
                    descendants.push_back( childObject );
                }
                std::list<ObjectHandle*> childsDescendants = childObject->matchingDescendants( predicate );
                descendants.insert( descendants.end(), childsDescendants.begin(), childsDescendants.end() );
            }
        }
    }
    return descendants;
}

std::list<ObjectHandle*> ObjectHandle::children() const
{
    std::list<ObjectHandle*> allChildren;
    for ( auto field : m_fields )
    {
        std::vector<ObjectHandle*> childObjects;
        field->childObjects( &childObjects );
        for ( auto childObject : childObjects )
        {
            allChildren.push_back( childObject );
        }
    }
    return allChildren;
}