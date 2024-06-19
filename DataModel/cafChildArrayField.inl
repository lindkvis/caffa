// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################
#include "cafLogger.h"

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

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to get object at " + std::to_string( index ) + " for '" + this->keyword() +
                                   "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

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

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to add object to '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    m_fieldDataAccessor->push_back( pointer );
}

//--------------------------------------------------------------------------------------------------
/// Assign a shared pointer
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::push_back_obj( std::shared_ptr<ObjectHandle> obj )
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

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to insert object at " + std::to_string( index ) + " in '" + this->keyword() +
                                   "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    m_fieldDataAccessor->insert( index, pointer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildArrayField<DataTypePtr>::insertAt( size_t index, std::shared_ptr<ObjectHandle> obj )
{
    auto typedPtr = std::dynamic_pointer_cast<DataType>( obj );
    CAFFA_ASSERT( typedPtr );
    if ( typedPtr )
    {
        this->insert( index, typedPtr );
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
    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to clear objects from '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }
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

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to remove object " + std::to_string( index ) + " from '" + this->keyword() +
                                   "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }
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
void ChildArrayField<DataTypePtr>::removeChildObject( std::shared_ptr<const ObjectHandle> object )
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to remove object from '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

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
std::vector<std::shared_ptr<ObjectHandle>> ChildArrayField<DataTypePtr>::childObjects()
{
    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to get child objects from '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }
    return m_fieldDataAccessor->objects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<std::shared_ptr<const ObjectHandle>> ChildArrayField<DataTypePtr>::childObjects() const
{
    const ChildArrayFieldAccessor* accessor = m_fieldDataAccessor.get();

    if ( !accessor )
    {
        std::string errorMessage = "Failed to get child objects from '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }
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
std::shared_ptr<ObjectHandle> ChildArrayField<DataTypePtr>::at( size_t index )
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to get object at " + std::to_string( index ) + " from '" + this->keyword() +
                                   "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    return m_fieldDataAccessor->at( index );
}

} // End of namespace caffa
