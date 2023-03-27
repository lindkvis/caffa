#include "cafObjectHandle.h"

#include "cafDynamicUniqueCast.h"

#include <iostream>
#include <vector>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ChildField<DataTypePtr>::~ChildField()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ChildField<DataTypePtr>& ChildField<DataTypePtr>::operator=( UniquePtr object )
{
    CAFFA_ASSERT( isInitialized() );
    this->setObject( std::move( object ) );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::setObject( UniquePtr object )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->setObject( std::move( object ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ChildField<DataTypePtr>::UniquePtr ChildField<DataTypePtr>::deepCloneObject() const
{
    CAFFA_ASSERT( isInitialized() );
    auto clonedObject = m_fieldDataAccessor->deepCloneObject();
    CAFFA_ASSERT( caffa::dynamic_unique_cast_is_valid<DataType>( clonedObject ) );
    return caffa::dynamic_unique_cast<DataType>( std::move( clonedObject ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::deepCopyObjectFrom( const DataType* copyFrom )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->deepCopyObjectFrom( copyFrom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<ObjectHandle*> ChildField<DataTypePtr>::childObjects()
{
    CAFFA_ASSERT( isInitialized() );

    auto object = m_fieldDataAccessor->object();
    if ( !object ) return {};

    return { object };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<const ObjectHandle*> ChildField<DataTypePtr>::childObjects() const
{
    CAFFA_ASSERT( isInitialized() );

    auto object = m_fieldDataAccessor->object();
    if ( !object ) return {};

    return { object };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::unique_ptr<ObjectHandle> ChildField<DataTypePtr>::clear()
{
    CAFFA_ASSERT( isInitialized() );
    return m_fieldDataAccessor->clear();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::unique_ptr<ObjectHandle> ChildField<DataTypePtr>::removeChildObject( ObjectHandle* object )
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
template <typename DataTypePtr>
void ChildField<DataTypePtr>::setChildObject( std::unique_ptr<ObjectHandle> object )
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
