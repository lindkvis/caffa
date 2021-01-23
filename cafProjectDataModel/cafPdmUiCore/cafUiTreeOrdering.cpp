//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafUiTreeOrdering.h"

#include "cafAssert.h"
#include "cafDataValueField.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"
#include "cafUiEditorHandle.h"

#include <iostream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::add( FieldHandle* field)
{
    CAF_ASSERT( field );

    if ( field->capability<FieldUiCapability>()->isUiTreeHidden( ) )
    {
        if ( !field->capability<FieldUiCapability>()->isUiTreeChildrenHidden() )
        {
            std::vector<ObjectHandle*> children;
            field->childObjects( &children );

            for ( ObjectHandle* objHandle : children )
            {
                this->add( objHandle );
            }
        }
    }
    else
    {
        new PdmUiTreeOrdering( this, field );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::add( ObjectHandle* object )
{
    CAF_ASSERT( object );

    new PdmUiTreeOrdering( this, object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiTreeOrdering::add( const std::string& title, const std::string& iconResourceName )
{
    PdmUiTreeOrdering* child = new PdmUiTreeOrdering( title, iconResourceName );
    CAF_ASSERT( child->isValid() );

    this->appendChild( child );
    return child;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeOrdering::containsField( const FieldHandle* field )
{
    CAF_ASSERT( field );
    for ( int cIdx = 0; cIdx < this->childCount(); ++cIdx )
    {
        PdmUiTreeOrdering* child = dynamic_cast<PdmUiTreeOrdering*>( this->child( cIdx ) ); // What ???

        if ( child->m_field == field )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeOrdering::containsObject( const ObjectHandle* object )
{
    CAF_ASSERT( object );
    for ( int cIdx = 0; cIdx < this->childCount(); ++cIdx )
    {
        PdmUiTreeOrdering* child = this->child( cIdx );

        if ( child->isRepresentingObject() && child->object() == object )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///  Creates an new root PdmUiTreeOrdering item, pointing at a Object
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::PdmUiTreeOrdering( ObjectHandle* pdmObject )
    : m_object( pdmObject )
    , m_field( nullptr )
    , m_uiItem( nullptr )
    , m_forgetRemainingFields( false )
    , m_isToIgnoreSubTree( false )
    , m_treeItemEditor( nullptr )
    , m_parentItem( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///  Creates an new root PdmUiTreeOrdering item, pointing at a field
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::PdmUiTreeOrdering( FieldHandle* pdmField )
    : m_object( nullptr )
    , m_field( pdmField )
    , m_uiItem( nullptr )
    , m_forgetRemainingFields( false )
    , m_isToIgnoreSubTree( false )
    , m_treeItemEditor( nullptr )
    , m_parentItem( nullptr )
{
    if ( pdmField ) m_object = pdmField->ownerObject();
}

//--------------------------------------------------------------------------------------------------
/// Creates an new root PdmUiTreeOrdering item, as a display item only
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::PdmUiTreeOrdering( const std::string& title, const std::string& iconResourceName )
    : m_object( nullptr )
    , m_field( nullptr )
    , m_uiItem( nullptr )
    , m_forgetRemainingFields( false )
    , m_isToIgnoreSubTree( false )
    , m_treeItemEditor( nullptr )
    , m_parentItem( nullptr )
{
    m_uiItem = std::make_unique<UiItem>();
    m_uiItem->setUiName( title );
    m_uiItem->setUiIconFromResourceString( iconResourceName );
}

//--------------------------------------------------------------------------------------------------
///  Creates an new PdmUiTreeOrdering item, and adds it to parent. If position is -1, it is added
///  at the end of parents existing child list.
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::PdmUiTreeOrdering( PdmUiTreeOrdering* parent, ObjectHandle* pdmObject )
    : m_object( pdmObject )
    , m_field( nullptr )
    , m_uiItem( nullptr )
    , m_forgetRemainingFields( false )
    , m_isToIgnoreSubTree( false )
    , m_treeItemEditor( nullptr )
    , m_parentItem( parent )
{
    if ( m_parentItem )
    {
        m_parentItem->m_childItems.push_back( this );
    }
}

//--------------------------------------------------------------------------------------------------
///  Creates an new PdmUiTreeOrdering item, and adds it to parent. If position is -1, it is added
///  at the end of parents existing child list.
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::PdmUiTreeOrdering( PdmUiTreeOrdering* parent, FieldHandle* pdmField )
    : m_object( nullptr )
    , m_field( pdmField )
    , m_forgetRemainingFields( false )
    , m_isToIgnoreSubTree( false )
    , m_parentItem( parent )
{
    if ( m_parentItem )
    {
        m_parentItem->m_childItems.push_back( this );
    }

    if ( pdmField ) m_object = pdmField->ownerObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering::~PdmUiTreeOrdering()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* PdmUiTreeOrdering::object() const
{
    CAF_ASSERT( isRepresentingObject() );
    return m_object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* PdmUiTreeOrdering::field() const
{
    CAF_ASSERT( isRepresentingField() );
    return m_field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* PdmUiTreeOrdering::uiItem() const
{
    CAF_ASSERT( isDisplayItemOnly() );
    return m_uiItem.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* PdmUiTreeOrdering::activeItem() const
{
    if ( isRepresentingObject() ) return uiObj( m_object );
    if ( isRepresentingField() ) return m_field->capability<FieldUiCapability>();
    if ( isDisplayItemOnly() ) return m_uiItem.get();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::setEditor( std::unique_ptr<UiEditorHandle> editor )
{
    m_treeItemEditor = std::move(editor);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiEditorHandle* PdmUiTreeOrdering::editor()
{
    return m_treeItemEditor.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::debugDump( int level ) const
{
    for ( int i = 0; i < level; ++i )
    {
        std::cout << "  ";
    }

    if ( isValid() )
    {
        char type = 'I';
        if ( isRepresentingObject() ) type = 'O';
        if ( isRepresentingField() ) type = 'F';
        if ( isDisplayItemOnly() ) type = 'D';

        std::cout << type << ": " << activeItem()->uiName() << std::endl;
    }
    else
    {
        std::cout << "NULL" << std::endl;
    }

    for ( int i = 0; i < childCount(); ++i )
    {
        child( i )->debugDump( level + 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiTreeOrdering::child( int index ) const
{
    CAF_ASSERT( index < m_childItems.size() );
    return m_childItems[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t PdmUiTreeOrdering::childCount() const
{
    return m_childItems.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiTreeOrdering::parent() const
{
    return m_parentItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t PdmUiTreeOrdering::indexInParent() const
{
    if ( m_parentItem )
    {
        auto it = std::find( m_parentItem->m_childItems.begin(), m_parentItem->m_childItems.end(), this );
        return it - m_parentItem->m_childItems.begin();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::appendChild( PdmUiTreeOrdering* child )
{
    m_childItems.push_back( child );
    child->m_parentItem = this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeOrdering::insertChild( int position, PdmUiTreeOrdering* child )
{
    m_childItems.insert( m_childItems.begin() + position, child );
    child->m_parentItem = this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeOrdering::removeChildren( int position, int count )
{
    if ( position < 0 || position + count > m_childItems.size() ) return false;

    for ( int row = 0; row < count; ++row )
    {
        PdmUiTreeOrdering* uiItem = m_childItems[position];
        m_childItems.erase( m_childItems.begin() + position );

        delete uiItem;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeOrdering::removeChildrenNoDelete( int position, int count )
{
    if ( position < 0 || position + count > m_childItems.size() ) return false;

    for ( int row = 0; row < count; ++row )
    {
        m_childItems.erase( m_childItems.begin() + position );
    }
    return true;
}

} // End of namespace caf
