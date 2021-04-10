#include "cafObjectHandle.h"

#include <iostream>
#include <vector>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::ChildField<DataType*>::childObjects( std::vector<ObjectHandle*>* objects )
{
    CAF_ASSERT( objects );
    ObjectHandle* obj = m_fieldValue.rawPtr();
    if ( obj )
    {
        objects->push_back( obj );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::ChildField<DataType*>::setChildObject( ObjectHandle* object )
{
    if ( m_fieldValue.rawPtr() != nullptr )
    {
        ObjectHandle* oldObject = m_fieldValue.rawPtr();
        this->removeChildObject( oldObject );
        delete oldObject;
    }
    m_fieldValue.setRawPtr( object );
    object->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::ChildField<DataType*>::removeChildObject( ObjectHandle* object )
{
    if ( m_fieldValue.rawPtr() != nullptr && m_fieldValue.rawPtr() == object )
    {
        m_fieldValue.rawPtr()->removeAsParentField( this );
        m_fieldValue.setRawPtr( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::ChildField<DataType*>::ChildField( const DataTypePtr& fieldValue )
{
    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::ChildField<DataType*>::~ChildField()
{
    delete m_fieldValue.rawPtr();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::ChildField<DataType*>& ChildField<DataType*>::operator=( const DataTypePtr& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::ChildField<DataType*>::setValue( const DataTypePtr& fieldValue )
{
    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

} // End of namespace caf
