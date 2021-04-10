#include "cafAsyncObjectDeleter.h"
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
    deleteAllChildObjects();
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
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::push_back( DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.push_back( pointer );
    if ( pointer ) pointer->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Set the value at position index to pointer, overwriting any pointer already present at that
/// position without deleting the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::set( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index] ) m_pointers[index]->removeAsParentField( this );
    m_pointers[index] = pointer;
    if ( m_pointers[index] ) pointer->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::insert( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, pointer );

    if ( pointer ) pointer->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Insert the pointers at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::insert( size_t index, const std::vector<Pointer<DataType>>& objects )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, objects.begin(), objects.end() );

    typename std::vector<Pointer<DataType>>::iterator it;
    for ( it = m_pointers.begin() + index; it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->setAsParentField( this );
        }
    }
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
/// Empty the container without deleting the objects pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::clear()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    this->removeThisAsParentField();
    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// Deletes all the objects pointed to by the field, then clears the container.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::deleteAllChildObjects()
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
/// Transfers ownership of the objects pointed to a separate thread.
/// Then clears the container and lets the thread delete the objects.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::deleteAllChildObjectsAsync()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    AsyncObjectVectorDeleter<DataType> pointerDeleter( m_pointers );
    CAF_ASSERT( m_pointers.empty() ); // Object storage for m_pointers should be empty immediately.
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container. Does not delete the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::erase( size_t index )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index].rawPtr() )
    {
        m_pointers[index].rawPtr()->removeAsParentField( this );
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
void ChildArrayField<DataType*>::setValue( const std::vector<DataType*>& objects )
{
    clear();
    for ( auto object : objects )
    {
        push_back( object );
    }
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    std::vector<Pointer<DataType>> tempPointers;

    tempPointers = m_pointers;
    m_pointers.clear();

    for ( size_t index = 0; index < tempPointers.size(); ++index )
    {
        if ( tempPointers[index].rawPtr() != object )
        {
            m_pointers.push_back( tempPointers[index] );
        }
        else
        {
            if ( tempPointers[index].rawPtr() )
            {
                tempPointers[index].rawPtr()->removeAsParentField( this );
            }
        }
    }
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
void ChildArrayField<DataType*>::insertAt( int indexAfter, ObjectHandle* obj )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    // This method should assert if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    typename std::vector<Pointer<DataType>>::iterator it;

    if ( indexAfter == -1 )
    {
        m_pointers.push_back( Pointer<DataType>() );
        it = m_pointers.end() - 1;
    }
    else
    {
        m_pointers.insert( m_pointers.begin() + indexAfter, Pointer<DataType>() );
        it = m_pointers.begin() + indexAfter;
    }

    it->setRawPtr( obj );
    obj->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ObjectHandle* ChildArrayField<DataType*>::at( size_t index )
{
    return m_pointers[index].rawPtr();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::removeThisAsParentField()
{
    typename std::vector<Pointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            it->rawPtr()->removeAsParentField( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::addThisAsParentField()
{
    typename std::vector<Pointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->setAsParentField( this );
        }
    }
}

} // End of namespace caf
