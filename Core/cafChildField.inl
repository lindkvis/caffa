#include "cafObjectHandle.h"

#include "cafDynamicUniqueCast.h"

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
    CAFFA_ASSERT( isInitialized() );
    this->setValue( std::move( fieldValue ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::setValue( DataTypePtr fieldValue )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->setValue( std::move( fieldValue ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<DataType> ChildField<DataType*>::cloneValue() const
{
    CAFFA_ASSERT( isInitialized() );
    auto clonedValue = m_fieldDataAccessor->cloneValue();
    CAFFA_ASSERT( caffa::dynamic_unique_cast_is_valid<DataType>( clonedValue ) );
    return caffa::dynamic_unique_cast<DataType>( std::move( clonedValue ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::copyValue( const DataType* copyFrom )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->copyValue( copyFrom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<ObjectHandle*> ChildField<DataType*>::childObjects() const
{
    CAFFA_ASSERT( isInitialized() );

    auto object = m_fieldDataAccessor->value();
    if ( !object ) return {};

    return { object };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildField<DataType*>::clear()
{
    CAFFA_ASSERT( isInitialized() );
    return m_fieldDataAccessor->clear();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAFFA_ASSERT( isInitialized() );
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
    CAFFA_ASSERT( isInitialized() );

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
