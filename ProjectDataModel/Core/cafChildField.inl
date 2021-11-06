#include "cafObjectHandle.h"

#include <iostream>
#include <vector>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>::ChildField( DataTypePtr fieldValue )
    : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>() )
{
    m_fieldDataAccessor->setValue( std::move( fieldValue ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>::~ChildField()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildField<DataType*>& ChildField<DataType*>::operator=( DataTypePtr fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    this->setValue( std::move( fieldValue ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::setValue( DataTypePtr fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    m_fieldDataAccessor->setValue( std::move( fieldValue ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<ObjectHandle*> ChildField<DataType*>::childObjects() const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    return { m_fieldDataAccessor->value() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildField<DataType*>::clear()
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    return m_fieldDataAccessor->clear();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    if ( m_fieldDataAccessor->value() == object )
    {
        return this->clear();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::setChildObject( std::unique_ptr<ObjectHandle> fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    ObjectHandle* rawPtr = fieldValue.release();

    DataType* typedPtr = dynamic_cast<DataType*>( rawPtr );
    if ( typedPtr )
    {
        m_fieldDataAccessor->setValue( std::unique_ptr<DataType>( typedPtr ) );
    }
    else
    {
        delete rawPtr;
    }
}

} // End of namespace caffa
