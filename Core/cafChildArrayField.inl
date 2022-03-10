#include "cafObjectHandle.h"

namespace caffa
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* ChildArrayField<DataType*>::operator[]( size_t index ) const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    return static_cast<DataType*>( m_fieldDataAccessor->at( index ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::push_back( DataTypeUniquePtr pointer )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    m_fieldDataAccessor->push_back( std::move( pointer ) );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::push_back_obj( std::unique_ptr<ObjectHandle> obj )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    ObjectHandle* rawObjPtr = obj.release();
    CAFFA_ASSERT( rawObjPtr );

    DataType* rawDataPtr = dynamic_cast<DataType*>( rawObjPtr );
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
template <typename DataType>
void ChildArrayField<DataType*>::insert( size_t index, DataTypeUniquePtr pointer )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    DataType* rawDataPtr = pointer.release();
    if ( rawDataPtr )
    {
        m_fieldDataAccessor->insert( index, std::unique_ptr<ObjectHandle>( rawDataPtr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::insertAt( size_t index, std::unique_ptr<ObjectHandle> obj )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    // This method should assert if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    ObjectHandle* rawObjPtr = obj.release();
    CAFFA_ASSERT( rawObjPtr );

    DataType* rawDataPtr = dynamic_cast<DataType*>( rawObjPtr );
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
template <typename DataType>
std::vector<std::unique_ptr<ObjectHandle>> ChildArrayField<DataType*>::clear()
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    return m_fieldDataAccessor->clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container and deletes the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::erase( size_t index )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    m_fieldDataAccessor->remove( index );
}

//--------------------------------------------------------------------------------------------------
/// Assign objects to the field, replacing the current child objects
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::setValue( std::vector<std::unique_ptr<DataType>>& objects )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

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
std::unique_ptr<ObjectHandle> ChildArrayField<DataType*>::removeChildObject( ObjectHandle* object )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

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
template <typename DataType>
std::vector<ObjectHandle*> caffa::ChildArrayField<DataType*>::childObjects() const
{
    return m_fieldDataAccessor->value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<DataType*> caffa::ChildArrayField<DataType*>::value() const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    std::vector<DataType*> typedObjects;
    for ( auto childObject : this->childObjects() )
    {
        typedObjects.push_back( static_cast<DataType*>( childObject ) );
    }

    return typedObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
ObjectHandle* ChildArrayField<DataType*>::at( size_t index )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    return m_fieldDataAccessor->at( index );
}

} // End of namespace caffa
