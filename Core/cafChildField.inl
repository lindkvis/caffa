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
ChildField<DataType*>::ChildField( DataTypePtr object )
    : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>() )
{
    m_fieldDataAccessor->setObject( std::move( object ) );
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
ChildField<DataType*>& ChildField<DataType*>::operator=( DataTypePtr object )
{
    CAFFA_ASSERT( isInitialized() );
    this->setObject( std::move( object ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::setObject( DataTypePtr object )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->setObject( std::move( object ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<DataType> ChildField<DataType*>::deepCloneObject() const
{
    CAFFA_ASSERT( isInitialized() );
    auto clonedObject = m_fieldDataAccessor->deepCloneObject();
    CAFFA_ASSERT( caffa::dynamic_unique_cast_is_valid<DataType>( clonedObject ) );
    return caffa::dynamic_unique_cast<DataType>( std::move( clonedObject ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::deepCopyObjectFrom( const DataType* copyFrom )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->deepCopyObjectFrom( copyFrom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<ObjectHandle*> ChildField<DataType*>::childObjects() const
{
    CAFFA_ASSERT( isInitialized() );

    auto object = m_fieldDataAccessor->object();
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
    if ( this->object() == object )
    {
        return this->clear();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildField<DataType*>::setChildObject( std::unique_ptr<ObjectHandle> object )
{
    CAFFA_ASSERT( isInitialized() );

    ObjectHandle* rawPtr = object.release();

    DataType* typedPtr = dynamic_cast<DataType*>( rawPtr );
    if ( typedPtr )
    {
        m_fieldDataAccessor->setObject( std::unique_ptr<DataType>( typedPtr ) );
    }
    else
    {
        delete rawPtr;
    }
}

} // End of namespace caffa
