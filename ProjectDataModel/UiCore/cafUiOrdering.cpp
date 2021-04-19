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

#include "cafUiOrdering.h"

#include "cafDataValueField.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiOrdering::~UiOrdering()
{
    for ( size_t i = 0; i < m_createdGroups.size(); ++i )
    {
        delete m_createdGroups[i];
        m_createdGroups[i] = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::addNewGroup( const std::string& displayName, LayoutOptions layout )
{
    UiGroup* group = new UiGroup;
    group->setUiName( displayName );

    m_createdGroups.push_back( group );
    m_ordering.push_back( std::make_pair( group, layout ) );

    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::addNewGroupWithKeyword( const std::string& displayName, const std::string& keyword, LayoutOptions layout )
{
    UiGroup* group = addNewGroup( displayName, layout );

    group->setKeyword( keyword );

    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiOrdering::insertBeforeGroup( const std::string& groupId, const FieldHandle* field, LayoutOptions layout )
{
    PositionFound pos = findGroupPosition( groupId );
    if ( pos.parent )
    {
        pos.parent->insert( pos.indexInParent, field, layout );
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiOrdering::insertBeforeItem( const UiItem* item, const FieldHandle* field, LayoutOptions layout )
{
    PositionFound pos = findItemPosition( item );
    if ( pos.parent )
    {
        pos.parent->insert( pos.indexInParent, field, layout );
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::createGroupBeforeGroup( const std::string& groupId, const std::string& displayName, LayoutOptions layout )
{
    return createGroupWithIdBeforeGroup( groupId, displayName, "", layout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::createGroupBeforeItem( const UiItem* item, const std::string& displayName, LayoutOptions layout )
{
    return createGroupWithIdBeforeItem( item, displayName, "", layout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::createGroupWithIdBeforeGroup( const std::string& groupId,
                                                   const std::string& displayName,
                                                   const std::string& newGroupId,
                                                   LayoutOptions      layout )
{
    PositionFound pos = findGroupPosition( groupId );
    if ( pos.parent )
    {
        return pos.parent->insertNewGroupWithKeyword( pos.indexInParent, displayName, newGroupId, layout );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::createGroupWithIdBeforeItem( const UiItem*      item,
                                                  const std::string& displayName,
                                                  const std::string& newGroupId,
                                                  LayoutOptions      layout )
{
    PositionFound pos = findItemPosition( item );
    if ( pos.parent )
    {
        return pos.parent->insertNewGroupWithKeyword( pos.indexInParent, displayName, newGroupId, layout );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::findGroup( const std::string& groupId ) const
{
    return findGroupPosition( groupId ).group();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::insertNewGroupWithKeyword( size_t             index,
                                                const std::string& displayName,
                                                const std::string& groupKeyword,
                                                LayoutOptions      layout )
{
    UiGroup* group = new UiGroup;
    group->setUiName( displayName );

    m_createdGroups.push_back( group );

    m_ordering.insert( m_ordering.begin() + index, std::make_pair( group, layout ) );

    group->setKeyword( groupKeyword );

    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiOrdering::contains( const UiItem* item ) const
{
    return this->findItemPosition( item ).parent != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiOrdering::PositionFound UiOrdering::findItemPosition( const UiItem* item ) const
{
    for ( size_t i = 0; i < m_ordering.size(); ++i )
    {
        if ( m_ordering[i].first == item ) return { const_cast<UiOrdering*>( this ), i };
        if ( m_ordering[i].first && m_ordering[i].first->isUiGroup() )
        {
            PositionFound result = static_cast<UiGroup*>( m_ordering[i].first )->findItemPosition( item );
            if ( result.parent ) return result;
        }
    }
    return { nullptr, size_t( -1 ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiOrdering::PositionFound UiOrdering::findGroupPosition( const std::string& groupKeyword ) const
{
    for ( size_t i = 0; i < m_ordering.size(); ++i )
    {
        if ( m_ordering[i].first && m_ordering[i].first->isUiGroup() )
        {
            if ( static_cast<UiGroup*>( m_ordering[i].first )->keyword() == groupKeyword )
                return { const_cast<UiOrdering*>( this ), i };
            PositionFound result = static_cast<UiGroup*>( m_ordering[i].first )->findGroupPosition( groupKeyword );
            if ( result.parent ) return result;
        }
    }
    return { nullptr, size_t( -1 ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiOrdering::add( const FieldHandle* field, LayoutOptions layout )
{
    FieldUiCapability* uiItem = const_cast<FieldHandle*>( field )->capability<FieldUiCapability>();
    CAFFA_ASSERT( uiItem );
    CAFFA_ASSERT( !this->contains( uiItem ) );

    m_ordering.push_back( std::make_pair( uiItem, layout ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiOrdering::add( const ObjectHandle* obj, LayoutOptions layout )
{
    ObjectUiCapability* uiItem = uiObj( const_cast<ObjectHandle*>( obj ) );
    CAFFA_ASSERT( uiItem );
    CAFFA_ASSERT( !this->contains( uiItem ) );
    m_ordering.push_back( std::make_pair( uiItem, layout ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiOrdering::insert( size_t index, const FieldHandle* field, LayoutOptions layout )
{
    FieldUiCapability* uiItem = const_cast<FieldHandle*>( field )->capability<FieldUiCapability>();
    CAFFA_ASSERT( uiItem );
    CAFFA_ASSERT( !this->contains( uiItem ) );

    m_ordering.insert( m_ordering.begin() + index, std::make_pair( uiItem, layout ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiOrdering::isIncludingRemainingFields() const
{
    return !m_skipRemainingFields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiOrdering::skipRemainingFields( bool doSkip /*= true*/ )
{
    m_skipRemainingFields = doSkip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<UiItem*> UiOrdering::uiItems() const
{
    std::vector<UiItem*> justUiItems;
    justUiItems.reserve( m_ordering.size() );
    for ( const FieldAndLayout& itemAndLayout : m_ordering )
    {
        justUiItems.push_back( itemAndLayout.first );
    }
    return justUiItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<UiOrdering::FieldAndLayout>& UiOrdering::uiItemsWithLayout() const
{
    return m_ordering;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiOrdering::TableLayout UiOrdering::calculateTableLayout() const
{
    TableLayout tableLayout;

    for ( size_t i = 0; i < m_ordering.size(); ++i )
    {
        if ( m_ordering[i].first->isUiHidden() ) continue;

        if ( m_ordering[i].second.newRow || i == 0u )
        {
            tableLayout.push_back( RowLayout() );
        }
        tableLayout.back().push_back( m_ordering[i] );
    }
    return tableLayout;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiOrdering::nrOfColumns( const TableLayout& tableLayout ) const
{
    int maxNrOfColumns = 0;

    for ( const auto& rowContent : tableLayout )
    {
        maxNrOfColumns = std::max( maxNrOfColumns, nrOfRequiredColumnsInRow( rowContent ) );
    }

    return maxNrOfColumns;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiOrdering::nrOfRequiredColumnsInRow( const RowLayout& rowItems ) const
{
    int totalColumns = 0;
    for ( const FieldAndLayout& item : rowItems )
    {
        int totalItemColumns = 0, labelItemColumns = 0, fieldItemColumns = 0;
        nrOfColumnsRequiredForItem( item, &totalItemColumns, &labelItemColumns, &fieldItemColumns );
        totalColumns += totalItemColumns;
    }
    return totalColumns;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiOrdering::nrOfExpandingItemsInRow( const RowLayout& rowItems ) const
{
    int nrOfExpandingItems = 0;
    for ( const FieldAndLayout& item : rowItems )
    {
        if ( item.second.totalColumnSpan == LayoutOptions::MAX_COLUMN_SPAN ) nrOfExpandingItems++;
    }
    return nrOfExpandingItems;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiOrdering::nrOfColumnsRequiredForItem( const FieldAndLayout& fieldAndLayout,
                                             int*                  totalColumnsRequired,
                                             int*                  labelColumnsRequired,
                                             int*                  fieldColumnsRequired ) const
{
    const UiItem* uiItem = fieldAndLayout.first;
    CAFFA_ASSERT( uiItem && totalColumnsRequired && labelColumnsRequired && fieldColumnsRequired );

    LayoutOptions layoutOption = fieldAndLayout.second;

    if ( uiItem->isUiGroup() )
    {
        *totalColumnsRequired = 1;
        *labelColumnsRequired = 0;
        *fieldColumnsRequired = 0;
    }
    else
    {
        *fieldColumnsRequired = 1;
        *labelColumnsRequired = 0;
        if ( uiItem->uiLabelPosition() == UiItemInfo::LEFT )
        {
            *labelColumnsRequired = 1;
            if ( layoutOption.leftLabelColumnSpan != LayoutOptions::MAX_COLUMN_SPAN )
            {
                *labelColumnsRequired = layoutOption.leftLabelColumnSpan;
            }
        }
        *totalColumnsRequired = *labelColumnsRequired + *fieldColumnsRequired;
    }

    if ( layoutOption.totalColumnSpan != LayoutOptions::MAX_COLUMN_SPAN )
    {
        *totalColumnsRequired = layoutOption.totalColumnSpan;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* UiOrdering::PositionFound::item()
{
    if ( parent )
    {
        return parent->uiItems()[indexInParent];
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiGroup* UiOrdering::PositionFound::group()
{
    UiItem* g = item();
    if ( g && g->isUiGroup() )
    {
        return static_cast<UiGroup*>( g );
    }
    else
    {
        return nullptr;
    }
}

} // End of namespace caffa
