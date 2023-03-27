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
ChildArrayField<DataTypePtr>::~ChildArrayField()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ChildArrayField<DataTypePtr>::DataType* ChildArrayField<DataTypePtr>::operator[]( size_t index ) const
{
    CAFFA_ASSERT( isInitialized() );
    return static_cast<DataTypePtr>( m_fieldDataAccessor->at( index ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::push_back( UniquePtr pointer )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->push_back( std::move( pointer ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::push_back_obj( std::unique_ptr<ObjectHandle> obj )
{
    CAFFA_ASSERT( isInitialized() );

    ObjectHandle* rawObjPtr = obj.release();
    CAFFA_ASSERT( rawObjPtr );

    DataType* rawDataPtr = dynamic_cast<DataTypePtr>( rawObjPtr );
    CAFFA_ASSERT( rawDataPtr );

    if ( rawDataPtr )
    {
        m_fieldDataAccessor->push_back( std::unique_ptr<DataType>( rawDataPtr ) );
    }
    else if ( rawObjPtr )
    {
        delete rawObjPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::insert( size_t index, UniquePtr pointer )
{
    CAFFA_ASSERT( isInitialized() );

    DataType* rawDataPtr = pointer.release();
    if ( rawDataPtr )
    {
        m_fieldDataAccessor->insert( index, std::unique_ptr<ObjectHandle>( rawDataPtr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::insertAt( size_t index, std::unique_ptr<ObjectHandle> obj )
{
    CAFFA_ASSERT( isInitialized() );

    // This method should assert if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    ObjectHandle* rawObjPtr = obj.release();
    CAFFA_ASSERT( rawObjPtr );

    DataType* rawDataPtr = dynamic_cast<DataTypePtr>( rawObjPtr );
    CAFFA_ASSERT( rawDataPtr );

    if ( rawDataPtr )
    {
        m_fieldDataAccessor->insert( index, std::unique_ptr<ObjectHandle>( rawDataPtr ) );
    }
    else if ( rawObjPtr )
    {
        delete rawObjPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/// Clears the container and returns a vector of unique_ptrs to the content
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<std::unique_ptr<ObjectHandle>> ChildArrayField<DataTypePtr>::clear()
{
    CAFFA_ASSERT( isInitialized() );

    return m_fieldDataAccessor->clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container and deletes the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::erase( size_t index )
{
    CAFFA_ASSERT( isInitialized() );
    m_fieldDataAccessor->remove( index );
}

//--------------------------------------------------------------------------------------------------
/// Assign objects to the field, replacing the current child objects
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
void ChildArrayField<DataTypePtr>::setObjects( std::vector<std::unique_ptr<DataType>>& objects )
{
    CAFFA_ASSERT( isInitialized() );

    clear();
    for ( auto& object : objects )
    {
        push_back( std::move( object ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::unique_ptr<ObjectHandle> ChildArrayField<DataTypePtr>::removeChildObject( ObjectHandle* object )
{
    CAFFA_ASSERT( isInitialized() );

    if ( object )
    {
        size_t index = m_fieldDataAccessor->index( object );
        if ( index < m_fieldDataAccessor->size() )
        {
            return m_fieldDataAccessor->remove( index );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<ObjectHandle*> ChildArrayField<DataTypePtr>::childObjects()
{
    return m_fieldDataAccessor->objects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<const ObjectHandle*> ChildArrayField<DataTypePtr>::childObjects() const
{
    const ChildArrayFieldAccessor* accessor = m_fieldDataAccessor.get();
    return accessor->objects();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<typename ChildArrayField<DataTypePtr>::DataType*> ChildArrayField<DataTypePtr>::objects()
{
    CAFFA_ASSERT( isInitialized() );

    std::vector<DataTypePtr> typedObjects;
    for ( auto childObject : this->childObjects() )
    {
        typedObjects.push_back( static_cast<DataTypePtr>( childObject ) );
    }

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
std::vector<const typename ChildArrayField<DataTypePtr>::DataType*> ChildArrayField<DataTypePtr>::objects() const
{
    CAFFA_ASSERT( isInitialized() );

    std::vector<const DataType*> typedObjects;
    for ( auto childObject : this->childObjects() )
    {
        typedObjects.push_back( static_cast<const DataType*>( childObject ) );
    }

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
ObjectHandle* ChildArrayField<DataTypePtr>::at( size_t index )
{
    CAFFA_ASSERT( isInitialized() );

    return m_fieldDataAccessor->at( index );
}

} // End of namespace caffa
