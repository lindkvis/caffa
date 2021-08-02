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
    clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* ChildArrayField<DataType*>::operator[]( size_t index ) const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    return m_fieldDataAccessor->at( index );
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
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::insert( size_t index, DataTypeUniquePtr pointer )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    m_fieldDataAccessor->insert( index, pointer );
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
        m_fieldDataAccessor->insert( index, std::unique_ptr<DataType>( rawDataPtr ) );
    }
    else if ( rawObjPtr )
    {
        delete rawObjPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/// Empty the container and delete all objects
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::clear()
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );
    m_fieldDataAccessor->removeAll();
}

//--------------------------------------------------------------------------------------------------
/// Clears the container and returns a vector of unique_ptrs to the content
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<std::unique_ptr<DataType>> ChildArrayField<DataType*>::removeAll()
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    return m_fieldDataAccessor->removeAll();
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
    std::vector<ObjectHandle*> objects;
    this->childObjects( &objects );
    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<DataType*> caffa::ChildArrayField<DataType*>::value() const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    return m_fieldDataAccessor->value();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void ChildArrayField<DataType*>::childObjects( std::vector<ObjectHandle*>* objects ) const
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    if ( !objects ) return;

    *objects = m_fieldDataAccessor->childObjects();
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
