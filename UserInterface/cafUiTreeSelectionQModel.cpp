//##################################################################################################
//
//   Custom Visualization Core library
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

#include "cafUiTreeSelectionQModel.h"

#include "cafObject.h"
#include "cafQVariantConverter.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiTreeViewQModel.h"

#include <QAbstractItemModel>
#include <QLabel>
#include <QTreeView>

#include <algorithm>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::UiTreeSelectionQModel::UiTreeSelectionQModel( QObject* parent /*= 0*/ )
    : QAbstractItemModel( parent )
    , m_uiFieldHandle( nullptr )
    , m_uiValueCache( nullptr )
    , m_tree( nullptr )
    , m_singleSelectionMode( false )
    , m_indexForLastUncheckedItem( QModelIndex() )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::UiTreeSelectionQModel::~UiTreeSelectionQModel()
{
    m_uiFieldHandle = nullptr;

    delete m_tree;
    m_tree = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caffa::UiTreeSelectionQModel::headingRole()
{
    return Qt::UserRole + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caffa::UiTreeSelectionQModel::optionItemValueRole()
{
    return Qt::UserRole + 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::setCheckedStateForItems( const QModelIndexList& sourceModelIndices, bool checked )
{
    if ( !m_uiFieldHandle || !m_uiFieldHandle->uiField() ) return;

    std::set<Variant> selectedItems;
    {
        Variant              fieldValue          = m_uiFieldHandle->uiField()->uiValue();
        std::vector<Variant> fieldValueSelection = fieldValue.toVector();

        for ( const auto& v : fieldValueSelection )
        {
            selectedItems.insert( v );
        }
    }

    if ( checked )
    {
        for ( const auto& mi : sourceModelIndices )
        {
            const caffa::OptionItemInfo* optionItemInfo = optionItem( mi );
            if ( !optionItemInfo->isReadOnly() )
            {
                selectedItems.insert( optionItem( mi )->value() );
            }
        }
    }
    else
    {
        for ( const auto& mi : sourceModelIndices )
        {
            const caffa::OptionItemInfo* optionItemInfo = optionItem( mi );
            if ( !optionItemInfo->isReadOnly() )
            {
                selectedItems.erase( optionItem( mi )->value() );
            }
        }
    }

    std::vector<Variant> fieldValueSelection;
    for ( const auto& v : selectedItems )
    {
        fieldValueSelection.push_back( v );
    }

    UiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(),
                                                         Variant::fromVector( fieldValueSelection ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::enableSingleSelectionMode( bool enable )
{
    m_singleSelectionMode = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t caffa::UiTreeSelectionQModel::optionItemCount() const
{
    return m_options.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::setOptions( caffa::UiFieldEditorHandle* field, const std::deque<caffa::OptionItemInfo>& options )
{
    m_uiFieldHandle = field;

    bool mustRebuildOptionItemTree = m_options.size() != options.size();

    m_options = options;

    if ( mustRebuildOptionItemTree )
    {
        beginResetModel();

        if ( m_tree )
        {
            delete m_tree;
            m_tree = nullptr;
        }

        m_tree = new TreeItemType( nullptr, -1, 0 );
        buildOptionItemTree( 0, m_tree );

        endResetModel();
    }
    else
    {
        // Notify changed for all items in the model as UI can change even if the option item count is identical
        // It is possible to use beginResetModel and endResetModel, but this will also invalidate tree expand state
        notifyChangedForAllModelIndices();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::setUiValueCache( const Variant& uiValuesCache )
{
    m_uiValueCache = uiValuesCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::resetUiValueCache()
{
    m_uiValueCache = Variant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::UiTreeSelectionQModel::isReadOnly( const QModelIndex& index ) const
{
    return optionItem( index )->isReadOnly();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::UiTreeSelectionQModel::isChecked( const QModelIndex& index ) const
{
    return data( index, Qt::CheckStateRole ).toBool();
}

//--------------------------------------------------------------------------------------------------
/// Checks if this is a real tree with grand children or just a list of children.
//--------------------------------------------------------------------------------------------------
bool caffa::UiTreeSelectionQModel::hasGrandChildren() const
{
    return m_tree && m_tree->hasGrandChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caffa::OptionItemInfo* caffa::UiTreeSelectionQModel::optionItem( const QModelIndex& index ) const
{
    int opIndex = optionIndex( index );

    return &m_options[opIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caffa::UiTreeSelectionQModel::optionIndex( const QModelIndex& index ) const
{
    CAFFA_ASSERT( index.isValid() );

    TreeItemType* item = static_cast<TreeItemType*>( index.internalPointer() );

    int optionIndex = item->dataObject();

    return optionIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags caffa::UiTreeSelectionQModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() )
    {
        const caffa::OptionItemInfo* optionItemInfo = optionItem( index );

        if ( !optionItemInfo->isHeading() )
        {
            if ( optionItemInfo->isReadOnly() )
            {
                return QAbstractItemModel::flags( index ) ^ Qt::ItemIsEnabled;
            }

            return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
        }
    }

    return QAbstractItemModel::flags( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caffa::UiTreeSelectionQModel::index( int row, int column, const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !hasIndex( row, column, parent ) ) return QModelIndex();

    TreeItemType* parentItem;

    if ( !parent.isValid() )
        parentItem = m_tree;
    else
        parentItem = static_cast<TreeItemType*>( parent.internalPointer() );

    TreeItemType* childItem = parentItem->child( row );
    if ( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caffa::UiTreeSelectionQModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caffa::UiTreeSelectionQModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() ) return QModelIndex();

    TreeItemType* childItem  = static_cast<TreeItemType*>( index.internalPointer() );
    TreeItemType* parentItem = childItem->parent();

    if ( parentItem == m_tree ) return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int caffa::UiTreeSelectionQModel::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
    if ( !m_tree ) return 0;

    if ( parent.column() > 0 ) return 0;

    TreeItemType* parentItem;
    if ( !parent.isValid() )
        parentItem = m_tree;
    else
        parentItem = static_cast<TreeItemType*>( parent.internalPointer() );

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant caffa::UiTreeSelectionQModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole*/ ) const
{
    if ( index.isValid() )
    {
        const caffa::OptionItemInfo* optionItemInfo = optionItem( index );

        if ( role == Qt::DisplayRole )
        {
            return QVariant( QString::fromStdString( optionItemInfo->optionUiText() ) );
        }
        else if ( role == Qt::DecorationRole )
        {
            auto icon = optionItemInfo->iconProvider();
            if ( icon && !icon->iconResourceString().empty() )
            {
                return QIcon( QString::fromStdString( icon->iconResourceString() ) );
            }
            return QIcon();
        }
        else if ( role == Qt::CheckStateRole && !optionItemInfo->isHeading() )
        {
            if ( m_uiFieldHandle && m_uiFieldHandle->uiField() )
            {
                // Avoid calling the seriously heavy uiValue method if we have a temporary valid cache.

                Variant fieldValue = m_uiValueCache.isValid() ? m_uiValueCache : m_uiFieldHandle->uiField()->uiValue();
                if ( !fieldValue.isVector() )
                {
                    if ( fieldValue == optionItem( index )->value() )
                    {
                        return Qt::Checked;
                    }
                }
                else
                {
                    std::vector<Variant> valuesSelectedInField = fieldValue.toVector();

                    auto currentOption = optionItem( index );

                    for ( const Variant& v : valuesSelectedInField )
                    {
                        if ( currentOption->value() == v )
                        {
                            return Qt::Checked;
                        }
                    }
                }
            }

            return Qt::Unchecked;
        }
        else if ( role == Qt::FontRole )
        {
            if ( optionItemInfo->isHeading() )
            {
                QFont font;
                font.setBold( true );

                return font;
            }
        }
        else if ( role == headingRole() )
        {
            return optionItemInfo->isHeading();
        }
        else if ( role == optionItemValueRole() )
        {
            Variant v = optionItemInfo->value();
            return QVariant::fromValue( v );
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caffa::UiTreeSelectionQModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
    if ( !m_uiFieldHandle || !m_uiFieldHandle->uiField() ) return false;

    if ( role == Qt::CheckStateRole )
    {
        Variant fieldValue = m_uiFieldHandle->uiField()->uiValue();
        if ( !fieldValue.isVector() )
        {
            if ( value.toBool() == true )
            {
                auto item = optionItem( index );
                if ( item )
                {
                    Variant v( item->value() );
                    UiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(), v );
                }
                return true;
            }
        }
        else
        {
            std::vector<Variant> previouslySelectedItems;

            if ( !m_singleSelectionMode )
            {
                previouslySelectedItems = fieldValue.toVector();
            }

            bool setSelected = value.toBool();

            // Do not allow empty selection in single selection mode
            if ( m_singleSelectionMode ) setSelected = true;

            auto opItem = optionItem( index );

            if ( setSelected && opItem )
            {
                bool isPresent = false;
                for ( auto previousItem : previouslySelectedItems )
                {
                    if ( previousItem == opItem->value() )
                    {
                        isPresent = true;
                    }
                }

                if ( !isPresent )
                {
                    previouslySelectedItems.push_back( opItem->value() );
                }
            }
            else
            {
                m_indexForLastUncheckedItem = index;

                previouslySelectedItems.erase( std::remove_if( previouslySelectedItems.begin(),
                                                               previouslySelectedItems.end(),
                                                               [&opItem]( const auto& v ) {
                                                                   return opItem->value() == v;
                                                               } ),
                                               previouslySelectedItems.end() );
            }

            Variant v = Variant::fromVector( previouslySelectedItems );

            UiCommandSystemProxy::instance()->setUiValueToField( m_uiFieldHandle->uiField(), v );
            emit dataChanged( index, index );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caffa::UiTreeSelectionQModel::indexForLastUncheckedItem() const
{
    return m_indexForLastUncheckedItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::clearIndexForLastUncheckedItem()
{
    m_indexForLastUncheckedItem = QModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::buildOptionItemTree( int parentOptionIndex, TreeItemType* parentNode )
{
    if ( parentNode == m_tree )
    {
        for ( int i = 0; i < m_options.size(); i++ )
        {
            if ( m_options[i].level() == 0 )
            {
                TreeItemType* node = new TreeItemType( parentNode, -1, i );

                buildOptionItemTree( i, node );
            }
        }
    }
    else
    {
        int currentOptionIndex = parentOptionIndex + 1;
        while ( currentOptionIndex < m_options.size() &&
                m_options[currentOptionIndex].level() > m_options[parentNode->dataObject()].level() )
        {
            if ( m_options[currentOptionIndex].level() == m_options[parentNode->dataObject()].level() + 1 )
            {
                TreeItemType* node = new TreeItemType( parentNode, -1, currentOptionIndex );

                buildOptionItemTree( currentOptionIndex, node );
            }
            currentOptionIndex++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::notifyChangedForAllModelIndices()
{
    recursiveNotifyChildren( QModelIndex() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caffa::UiTreeSelectionQModel::recursiveNotifyChildren( const QModelIndex& index )
{
    for ( int r = 0; r < rowCount( index ); r++ )
    {
        QModelIndex mi = this->index( r, 0, index );
        recursiveNotifyChildren( mi );
    }

    if ( index.isValid() )
    {
        emit dataChanged( index, index );
    }
}
