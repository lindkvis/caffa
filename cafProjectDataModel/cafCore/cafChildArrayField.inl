#include "cafObjectHandle.h"

namespace caf
{
//==================================================================================================
/// Implementation of ChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ChildArrayField<DataType*>::~ChildArrayField()
{
    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* ChildArrayField<DataType*>::operator[]( size_t index ) const
{
    return m_pointers[index];
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
/// This should be preferred over the method taking a raw pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
Pointer<DataType> ChildArrayField<DataType*>::push_back( DataTypeUniquePtr pointer )
{
    Pointer<DataType> ptr( pointer.release() );
    ptr->setAsParentField( this );
    m_pointers.push_back( ptr );
    return ptr;
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
Pointer<DataType> ChildArrayField<DataType*>::insert( size_t index, DataTypeUniquePtr pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    Pointer<DataType> rawPtr( pointer.release() );
    rawPtr->setAsParentField( this );
    m_pointers.insert( m_pointers.begin() + index, rawPtr );

    return rawPtr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
Pointer<ObjectHandle> ChildArrayField<DataType*>::insertAt( size_t index, std::unique_ptr<ObjectHandle> obj )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    // This method should assert if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    ObjectHandle*     rawObjectHandle = obj.release();
    Pointer<DataType> rawPtr( dynamic_cast<DataType*>( rawObjectHandle ) );

    if ( rawPtr.notNull() )
    {
        rawPtr->setAsParentField( this );
        m_pointers.insert( m_pointers.begin() + index, rawPtr );
        return Pointer<ObjectHandle>( rawObjectHandle );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Returns the number of times pointer is referenced from the container.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t ChildArrayField<DataType*>::count( const DataType* pointer ) const
{
    size_t itemCount = 0;

    typename std::vector<Pointer<DataType>>::const_iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( *it == pointer )
        {
            itemCount++;
        }
    }

    return itemCount;
}

//--------------------------------------------------------------------------------------------------
/// Empty the container and delete all objects
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::clear()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    size_t index;
    for ( index = 0; index < m_pointers.size(); ++index )
    {
        delete ( m_pointers[index].rawPtr() );
    }
    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// Clears the container and returns a vector of unique_ptrs to the content
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<std::unique_ptr<DataType>> ChildArrayField<DataType*>::removeAll()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    std::vector<Pointer<DataType>> tempPointers;
    tempPointers.swap( m_pointers );

    std::vector<std::unique_ptr<DataType>> uniquePointers;
    for ( auto pointer : tempPointers )
    {
        if ( !pointer.isNull() )
        {
            pointer->removeAsParentField( this );
            uniquePointers.push_back( std::unique_ptr<DataType>( pointer.p() ) );
        }
    }
    return uniquePointers;
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container and deletes the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::erase( size_t index )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );
    CAF_ASSERT( index < m_pointers.size() );

    auto rawPtr = m_pointers[index].rawPtr();
    if ( rawPtr )
    {
        rawPtr->removeAsParentField( this );
        delete rawPtr;
    }

    m_pointers.erase( m_pointers.begin() + index );
}

//--------------------------------------------------------------------------------------------------
/// Get the index of the given object pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t ChildArrayField<DataType*>::index( const DataType* pointer ) const
{
    for ( size_t i = 0; i < m_pointers.size(); ++i )
    {
        if ( pointer == m_pointers[i].p() )
        {
            return i;
        }
    }

    return ( size_t )( -1 ); // Undefined size_t > m_pointers.size();
}

//--------------------------------------------------------------------------------------------------
/// Assign objects to the field, replacing the current child objects
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::setValue( const std::vector<std::unique_ptr<DataType>>& objects )
{
    clear();
    for ( auto& object : objects )
    {
        push_back( std::move( object ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<DataType> ChildArrayField<DataType*>::remove( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    for ( auto it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        auto ptr = it->p();
        if ( ptr == object )
        {
            ptr->removeAsParentField( this );
            m_pointers.erase( it );
            return std::unique_ptr<DataType>( ptr );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::unique_ptr<ObjectHandle> ChildArrayField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    for ( auto it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        auto ptr = it->p();
        if ( ptr == object )
        {
            ptr->removeAsParentField( this );
            m_pointers.erase( it );
            return std::unique_ptr<ObjectHandle>( ptr );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<DataType*> caf::ChildArrayField<DataType*>::childObjects() const
{
    std::vector<DataType*> objects;

    for ( DataType* p : m_pointers )
    {
        if ( p != nullptr )
        {
            objects.push_back( p );
        }
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::childObjects( std::vector<ObjectHandle*>* objects )
{
    if ( !objects ) return;
    size_t i;
    for ( i = 0; i < m_pointers.size(); ++i )
    {
        objects->push_back( m_pointers[i].rawPtr() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ObjectHandle* ChildArrayField<DataType*>::at( size_t index )
{
    return m_pointers[index].rawPtr();
}

} // End of namespace caf
