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
#include "cafObjectHandle.h"

#include <iostream>
#include <vector>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//------------------------------------- -------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
ChildField<DataTypePtr>::~ChildField()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
ChildField<DataTypePtr>& ChildField<DataTypePtr>::operator=( std::shared_ptr<DataType> object )
{
    CAFFA_ASSERT( isInitialized() );
    this->setObject( object );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
bool ChildField<DataTypePtr>::operator==( ObjectHandle::ConstPtr object ) const
{
    return this->object() == object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
bool ChildField<DataTypePtr>::operator==( const ObjectHandle* object ) const
{
    return this->object().get() == object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildField<DataTypePtr>::setObject( std::shared_ptr<DataType> object )
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to set object for '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    m_fieldDataAccessor->setObject( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<ObjectHandle::Ptr> ChildField<DataTypePtr>::childObjects()
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to get child objects for '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    auto object = m_fieldDataAccessor->object();
    if ( !object ) return {};

    return { object };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
std::vector<ObjectHandle::ConstPtr> ChildField<DataTypePtr>::childObjects() const
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to get child objects for '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    auto object = m_fieldDataAccessor->object();
    if ( !object ) return {};

    return { object };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildField<DataTypePtr>::clear()
{
    CAFFA_ASSERT( isInitialized() );
    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to clear object for '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    m_fieldDataAccessor->clear();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
void ChildField<DataTypePtr>::removeChildObject( ObjectHandle::ConstPtr object )
{
    CAFFA_ASSERT( isInitialized() );

    if ( !m_fieldDataAccessor )
    {
        std::string errorMessage = "Failed to remove object for '" + this->keyword() + "': Field is not accessible";
        CAFFA_ERROR( errorMessage );
        throw std::runtime_error( errorMessage );
    }

    if ( this->object() == object )
    {
        this->clear();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
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
