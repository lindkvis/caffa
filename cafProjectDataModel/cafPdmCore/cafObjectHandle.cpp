
#include "cafObjectHandle.h"

#include "cafAssert.h"
#include "cafChildArrayField.h"
#include "cafFieldHandle.h"
#include "cafObjectCapability.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::ObjectHandle()
{
    m_parentField = nullptr;
    m_isDeletable = false;
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
QString ObjectHandle::classKeywordStatic()
{
    return classKeywordAliases().front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> ObjectHandle::classKeywordAliases()
{
    return { QString( "ObjectHandle" ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::fields( std::vector<FieldHandle*>& fields ) const
{
    fields = m_fields;
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
void ObjectHandle::addField( FieldHandle* field, const QString& keyword )
{
    field->m_ownerObject = this;

    CAF_ASSERT( !keyword.isEmpty() );
    CAF_ASSERT( this->findField( keyword ) == nullptr );

    field->setKeyword( keyword );
    m_fields.push_back( field );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ObjectHandle::findField( const QString& keyword ) const
{
    std::vector<FieldHandle*> fields;
    this->fields( fields );

    for ( size_t it = 0; it < fields.size(); it++ )
    {
        FieldHandle* field = fields[it];
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
void ObjectHandle::setDeletable( bool isDeletable )
{
    m_isDeletable = isDeletable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectHandle::isDeletable() const
{
    return m_isDeletable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::onChildDeleted( ChildArrayFieldHandle*           childArray,
                                      std::vector<caf::ObjectHandle*>& referringObjects )
{
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
ObjectUiCapability* ObjectHandle::capability<FieldUiCapability>()
{
return NULL;
}

PdmXmlObjectHandle* ObjectHandle::capability<ObjectXmlCapability>()
{
return NULL;
}
*/

} // End namespace caf
