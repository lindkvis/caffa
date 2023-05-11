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
ChildField<DataTypePtr>& ChildField<DataTypePtr>::operator=( Ptr object )
{
    CAFFA_ASSERT( isInitialized() );
    this->setObject( object );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
bool ChildField<DataTypePtr>::operator==( ObjectHandle::ConstPtr object ) const
{
    return this->object() == object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
bool ChildField<DataTypePtr>::operator==( const ObjectHandle* object ) const
{
    return this->object().get() == object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::setObject( Ptr object )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->setObject( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ChildField<DataTypePtr>::Ptr ChildField<DataTypePtr>::deepCloneObject() const
{
    CAFFA_ASSERT( isInitialized() );
    auto clonedObject = m_fieldDataAccessor->deepCloneObject();
    return std::dynamic_pointer_cast<DataType>( clonedObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::deepCopyObjectFrom( std::shared_ptr<const DataType> copyFrom )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->deepCopyObjectFrom( copyFrom );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<ObjectHandle::Ptr> ChildField<DataTypePtr>::childObjects()
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
std::vector<ObjectHandle::ConstPtr> ChildField<DataTypePtr>::childObjects() const
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
void ChildField<DataTypePtr>::clear()
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->clear();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::removeChildObject( ObjectHandle::ConstPtr object )
{
    CAFFA_ASSERT( isInitialized() );
    if ( this->object() == object )
    {
        this->clear();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildField<DataTypePtr>::setChildObject( ObjectHandle::Ptr object )
{
    CAFFA_ASSERT( isInitialized() );

    auto typedObject = std::dynamic_pointer_cast<DataType>( object );
    if ( typedObject )
    {
        this->setObject( typedObject );
    }
}

} // End of namespace caffa
