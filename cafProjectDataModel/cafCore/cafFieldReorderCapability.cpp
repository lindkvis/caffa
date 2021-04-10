//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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
//##################################################################################################

#include "cafFieldReorderCapability.h"
#include "cafObjectHandle.h"

#include "cafAssert.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldReorderCapability::FieldReorderCapability( PtrArrayFieldHandle* field, bool giveOwnership )
    : orderChanged( this )
    , m_field( field )

{
    field->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::canItemBeMovedUp( size_t index ) const
{
    return index != 0u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::canItemBeMovedDown( size_t index ) const
{
    return m_field->size() > 1u && index < m_field->size() - 1u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::moveItemUp( size_t index )
{
    if ( canItemBeMovedUp( index ) )
    {
        ObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            size_t newIndex = index - 1u;
            m_field->erase( index );
            m_field->insertAt( (int)newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::moveItemDown( size_t index )
{
    if ( canItemBeMovedDown( index ) )
    {
        ObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            size_t newIndex = index + 1u;
            m_field->erase( index );
            m_field->insertAt( (int)newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldReorderCapability* FieldReorderCapability::addToField( PtrArrayFieldHandle* field )
{
    if ( !fieldIsReorderable( field ) )
    {
        new FieldReorderCapability( field, true );
    }
    return field->capability<FieldReorderCapability>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::fieldIsReorderable( PtrArrayFieldHandle* field )
{
    return field->capability<FieldReorderCapability>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldReorderCapability::onMoveItemUp( const SignalEmitter* emitter, size_t index )
{
    moveItemUp( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldReorderCapability::onMoveItemDown( const SignalEmitter* emitter, size_t index )
{
    moveItemDown( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldReorderCapability::onMoveItemToTop( const SignalEmitter* emitter, size_t index )
{
    moveItemToTop( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldReorderCapability::moveItemToTop( size_t index )
{
    if ( canItemBeMovedUp( index ) )
    {
        ObjectHandle* itemToShift = m_field->at( index );
        if ( itemToShift )
        {
            int newIndex = 0;
            m_field->erase( index );
            m_field->insertAt( newIndex, itemToShift );
            orderChanged.send();
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldReorderCapability* FieldReorderCapability::reorderCapabilityOfParentContainer( const ObjectHandle* object )
{
    if ( object )
    {
        PtrArrayFieldHandle* arrayField = dynamic_cast<PtrArrayFieldHandle*>( object->parentField() );
        if ( arrayField )
        {
            FieldReorderCapability* reorderability = arrayField->capability<FieldReorderCapability>();
            return reorderability;
        }
    }

    return nullptr;
}
