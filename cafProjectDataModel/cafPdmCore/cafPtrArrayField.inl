#pragma once

#include "cafObjectHandle.h"

namespace caf
{
//==================================================================================================
/// Implementation of PtrArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
PtrArrayField<DataType*>::~PtrArrayField()
{
    this->removeThisAsReferencingPtrField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::setValue( const std::vector<PdmPointer<DataType>>& fieldValue )
{
    this->clear();
    this->insert( 0, fieldValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
const std::vector<PdmPointer<DataType>>& PtrArrayField<DataType*>::value() const
{
    return m_pointers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::setValue( const std::vector<DataType*>& fieldValue )
{
    this->clear();
    for ( DataType* rawPtr : fieldValue )
    {
        this->push_back( PdmPointer<DataType>( rawPtr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* PtrArrayField<DataType*>::operator[]( size_t index ) const
{
    return m_pointers[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::push_back( DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.push_back( pointer );
    if ( pointer ) pointer->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
/// Set the value at position index to pointer, overwriting any pointer already present at that
/// position without deleting the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::set( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index] ) m_pointers[index]->removeReferencingPtrField( this );
    m_pointers[index] = pointer;
    if ( m_pointers[index] ) pointer->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::insert( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, pointer );

    if ( pointer ) pointer->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
/// Insert the pointers at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::insert( size_t index, const std::vector<PdmPointer<DataType>>& objects )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, objects.begin(), objects.end() );

    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin() + index; it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->addReferencingPtrField( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the number of times pointer is referenced from the container.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t PtrArrayField<DataType*>::count( const DataType* pointer ) const
{
    size_t itemCount = 0;

    typename std::vector<PdmPointer<DataType>>::const_iterator it;
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
void PtrArrayField<DataType*>::clear()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    this->removeThisAsReferencingPtrField();
    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container. Does not delete the object pointed to.
/// Todo: This implementation can't be necessary in the new regime. Should be to just remove
/// the item at index (shrinking the array)
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::erase( size_t index )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index].rawPtr() )
    {
        m_pointers[index].rawPtr()->removeReferencingPtrField( this );
    }

    m_pointers.erase( m_pointers.begin() + index );
}

//--------------------------------------------------------------------------------------------------
/// Get the index of the given object pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t PtrArrayField<DataType*>::index( DataType* pointer )
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
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::removePtr( ObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    std::vector<PdmPointer<DataType>> tempPointers;

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
                tempPointers[index].rawPtr()->removeReferencingPtrField( this );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<DataType*> PtrArrayField<DataType*>::ptrReferencedObjects() const
{
    std::vector<DataType*> objects;

    size_t i;
    for ( i = 0; i < m_pointers.size(); ++i )
    {
        objects.push_back( m_pointers[i].p() );
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::ptrReferencedObjects( std::vector<ObjectHandle*>* objects )
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
void PtrArrayField<DataType*>::insertAt( int indexAfter, ObjectHandle* obj )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    // This method should CAF_ASSERT( if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    typename std::vector<PdmPointer<DataType>>::iterator it;

    if ( indexAfter == -1 )
    {
        m_pointers.push_back( PdmPointer<DataType>() );
        it = m_pointers.end() - 1;
    }
    else
    {
        m_pointers.insert( m_pointers.begin() + indexAfter, PdmPointer<DataType>() );
        it = m_pointers.begin() + indexAfter;
    }

    it->setRawPtr( obj );
    obj->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ObjectHandle* PtrArrayField<DataType*>::at( size_t index )
{
    return m_pointers[index].rawPtr();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::removeThisAsReferencingPtrField()
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            it->rawPtr()->removeReferencingPtrField( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrArrayField<DataType*>::addThisAsReferencingPtrField()
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->addReferencingPtrField( this );
        }
    }
}

} // End of namespace caf
