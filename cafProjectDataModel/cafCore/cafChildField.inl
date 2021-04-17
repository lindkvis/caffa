#include "cafObjectHandle.h"

#include <iostream>
#include <vector>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::childObjects( std::vector<ObjectHandle*>* objects )
{
    CAF_ASSERT( objects );
    CAF_ASSERT( isInitializedByInitFieldMacro() );
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
std::unique_ptr<DataType> ChildField<DataType*>::remove( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );
    if ( m_fieldValue.rawPtr() != nullptr && m_fieldValue.rawPtr() == object )
    {
        auto typedObject = m_fieldValue.p();
        m_fieldValue.rawPtr()->removeAsParentField( this );
        m_fieldValue.setRawPtr( nullptr );
        return std::unique_ptr<DataType>( typedObject );
    }
    return nullptr;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );
    if ( m_fieldValue.rawPtr() != nullptr && m_fieldValue.rawPtr() == object )
    {
        auto typedObject = m_fieldValue.p();
        m_fieldValue.rawPtr()->removeAsParentField( this );
        m_fieldValue.setRawPtr( nullptr );
        return std::unique_ptr<ObjectHandle>( typedObject );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>::ChildField( DataTypePtr fieldValue )
{
    m_fieldValue = fieldValue.release();
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>::~ChildField()
{
    delete m_fieldValue.rawPtr();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>& ChildField<DataType*>::operator=( DataTypePtr fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue )
    {
        m_fieldValue->removeAsParentField( this );
        delete m_fieldValue.rawPtr();
    }
    m_fieldValue = fieldValue.release();
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* ChildField<DataType*>::setValue( DataTypePtr fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );
    if ( m_fieldValue )
    {
        m_fieldValue->removeAsParentField( this );
        delete m_fieldValue.rawPtr();
    }
    m_fieldValue = fieldValue.release();
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );

    return m_fieldValue;
}

} // End of namespace caf
