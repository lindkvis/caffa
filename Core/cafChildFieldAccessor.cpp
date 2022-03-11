//##################################################################################################
//
//   Caffa
//   Copyright (C) 3D-Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#include "cafChildFieldAccessor.h"

#include "cafObjectHandle.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ChildFieldDirectStorageAccessor::ChildFieldDirectStorageAccessor( FieldHandle* field )
    : ChildFieldAccessor( field )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ChildFieldDirectStorageAccessor::value() const
{
    return m_value.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ChildFieldDirectStorageAccessor::setValue( std::unique_ptr<ObjectHandle> value )
{
    if ( m_value )
    {
        m_value->detachFromParentField();
    }
    if ( value )
    {
        value->setAsParentField( this->m_field );
    }
    m_value = std::move( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ChildFieldDirectStorageAccessor::clear()
{
    return std::move( m_value );
}