#include "cafObjectHandle.h"

namespace caffa
{
//==================================================================================================
/// Implementation of ChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
ChildArrayField<DataTypePtr>::~ChildArrayField()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::shared_ptr<typename ChildArrayField<DataTypePtr>::DataType> ChildArrayField<DataTypePtr>::operator[]( size_t index ) const
{
    CAFFA_ASSERT( isInitialized() );
    return std::dynamic_pointer_cast<DataType>( m_fieldDataAccessor->at( index ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a shared pointer
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::push_back( std::shared_ptr<DataType> pointer )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->push_back( pointer );
}

//--------------------------------------------------------------------------------------------------
/// Assign a shared pointer
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::push_back_obj( ObjectHandle::Ptr obj )
{
    CAFFA_ASSERT( isInitialized() );

    auto typedObject = std::dynamic_pointer_cast<DataType>( obj );
    if ( typedObject )
    {
        this->push_back( typedObject );
    }
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::insert( size_t index, std::shared_ptr<DataType> pointer )
{
    CAFFA_ASSERT( isInitialized() );

    m_fieldDataAccessor->insert( index, pointer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::insertAt( size_t index, ObjectHandle::Ptr obj )
{
    CAFFA_ASSERT( isInitialized() );

    auto typedPtr = std::dynamic_pointer_cast<DataType>( obj );
    CAFFA_ASSERT( typedPtr );
    if ( typedPtr )
    {
        m_fieldDataAccessor->insert( index, obj );
    }
}

//--------------------------------------------------------------------------------------------------
/// Clears the container
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::clear()
{
    CAFFA_ASSERT( isInitialized() );

    return m_fieldDataAccessor->clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container and deletes the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::erase( size_t index )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->remove( index );
}

//--------------------------------------------------------------------------------------------------
/// Assign objects to the field, replacing the current child objects
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::setObjects( std::vector<std::shared_ptr<DataType>>& objects )
{
    CAFFA_ASSERT( isInitialized() );

    clear();
    for ( auto object : objects )
    {
        push_back( object );
    }
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::removeChildObject( ObjectHandle::ConstPtr object )
{
    CAFFA_ASSERT( isInitialized() );

    if ( object )
    {
        size_t index = m_fieldDataAccessor->index( object );
        if ( index < m_fieldDataAccessor->size() )
        {
            m_fieldDataAccessor->remove( index );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<ObjectHandle::Ptr> ChildArrayField<DataTypePtr>::childObjects()
{
    return m_fieldDataAccessor->objects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<ObjectHandle::ConstPtr> ChildArrayField<DataTypePtr>::childObjects() const
{
    const ChildArrayFieldAccessor* accessor = m_fieldDataAccessor.get();
    return accessor->objects();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<std::shared_ptr<typename ChildArrayField<DataTypePtr>::DataType>> ChildArrayField<DataTypePtr>::objects()
{
    CAFFA_ASSERT( isInitialized() );

    std::vector<std::shared_ptr<DataType>> typedObjects;
    for ( auto childObject : this->childObjects() )
    {
        typedObjects.push_back( std::dynamic_pointer_cast<DataType>( childObject ) );
    }

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<std::shared_ptr<const typename ChildArrayField<DataTypePtr>::DataType>>
    ChildArrayField<DataTypePtr>::objects() const
{
    CAFFA_ASSERT( isInitialized() );

    std::vector<ConstPtr> typedObjects;
    for ( auto childObject : this->childObjects() )
    {
        typedObjects.push_back( std::dynamic_pointer_cast<const DataType>( childObject ) );
    }

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
ObjectHandle::Ptr ChildArrayField<DataTypePtr>::at( size_t index )
{
    CAFFA_ASSERT( isInitialized() );

    return m_fieldDataAccessor->at( index );
}

} // End of namespace caffa
