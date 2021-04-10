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
#pragma once

#include "cafSelectionChangedReceiver.h"

#include "cafField.h"
#include "cafObjectHandle.h"
#include "cafPointer.h"
#include "cafUiItem.h"

#include <set>
#include <string>
#include <vector>

namespace caf
{
class ChildArrayFieldHandle;

//==================================================================================================
///
//==================================================================================================
class SelectionManager
{
public:
    enum SelectionLevel
    {
        UNDEFINED    = -1,
        BASE_LEVEL   = 0,
        FIRST_LEVEL  = 1,
        SECOND_LEVEL = 2
    };

public:
    static SelectionManager* instance();

    UiItem* selectedItem( int selectionLevel = 0 );
    void    selectedItems( std::vector<UiItem*>& items, int selectionLevel = 0 );

    void setSelectedItem( UiItem* item );
    void setSelectedItemAtLevel( UiItem* item, int selectionLevel );

    void setSelectedItems( const std::vector<UiItem*>& items );
    void setSelectedItemsAtLevel( const std::vector<UiItem*>& items, int selectionLevel = 0 );

    struct SelectionItem
    {
        UiItem* item;
        int     selectionLevel;
    };
    void setSelection( const std::vector<SelectionItem> completeSelection );

    void selectionAsReferences( std::vector<std::string>& referenceList, int selectionLevel = 0 ) const;
    void setSelectionAtLevelFromReferences( const std::vector<std::string>& referenceList, int selectionLevel );

    bool isSelected( UiItem* item, int selectionLevel ) const;

    void clearAll();
    void clear( int selectionLevel );
    void removeObjectFromAllSelections( ObjectHandle* pdmObject );

    template <typename T>
    void objectsByType( std::vector<T*>* typedObjects, int selectionLevel = 0 )
    {
        std::vector<UiItem*> items;
        this->selectedItems( items, selectionLevel );
        for ( size_t i = 0; i < items.size(); i++ )
        {
            T* obj = dynamic_cast<T*>( items[i] );
            if ( obj ) typedObjects->push_back( obj );
        }
    }

    /// Returns the selected objects of the requested type if _all_ the selected objects are of the requested type

    template <typename T>
    void objectsByTypeStrict( std::vector<T*>* typedObjects, int selectionLevel = 0 )
    {
        std::vector<UiItem*> items;
        this->selectedItems( items, selectionLevel );
        for ( size_t i = 0; i < items.size(); i++ )
        {
            T* obj = dynamic_cast<T*>( items[i] );
            if ( !obj )
            {
                typedObjects->clear();
                break;
            }
            typedObjects->push_back( obj );
        }
    }

    template <typename T>
    T* selectedItemOfType( int selectionLevel = 0 )
    {
        std::vector<T*> typedObjects;
        this->objectsByType<T>( &typedObjects, selectionLevel );
        if ( !typedObjects.empty() )
        {
            return typedObjects.front();
        }
        return nullptr;
    }

    template <typename T>
    T* selectedItemAncestorOfType( int selectionLevel = 0 )
    {
        UiItem*       item           = this->selectedItem( selectionLevel );
        ObjectHandle* selectedObject = dynamic_cast<ObjectHandle*>( item );
        if ( selectedObject )
        {
            T* ancestor = nullptr;
            selectedObject->firstAncestorOrThisOfType( ancestor );
            return ancestor;
        }
        return nullptr;
    }

    void                   setActiveChildArrayFieldHandle( ChildArrayFieldHandle* childArray );
    ChildArrayFieldHandle* activeChildArrayFieldHandle();

    void          setPdmRootObject( ObjectHandle* root );
    ObjectHandle* pdmRootObject() { return m_rootObject; }

private:
    SelectionManager();

    static void
        extractInternalSelectionItems( const std::vector<UiItem*>&                             items,
                                       std::vector<std::pair<Pointer<ObjectHandle>, UiItem*>>* internalSelectionItems );

    void          notifySelectionChanged( const std::set<int>& changedSelectionLevels );
    std::set<int> findChangedLevels(
        const std::map<int, std::vector<std::pair<Pointer<ObjectHandle>, UiItem*>>>& newCompleteSelectionMap ) const;

    friend class SelectionChangedReceiver;
    void registerSelectionChangedReceiver( SelectionChangedReceiver* receiver )
    {
        m_selectionReceivers.insert( receiver );
    }
    void unregisterSelectionChangedReceiver( SelectionChangedReceiver* receiver )
    {
        m_selectionReceivers.erase( receiver );
    }

private:
    std::map<int, std::vector<std::pair<Pointer<ObjectHandle>, UiItem*>>> m_selectionPrLevel;

    ChildArrayFieldHandle* m_activeChildArrayFieldHandle;
    Pointer<ObjectHandle>  m_rootObject;

    std::set<SelectionChangedReceiver*> m_selectionReceivers;
};

} // end namespace caf
