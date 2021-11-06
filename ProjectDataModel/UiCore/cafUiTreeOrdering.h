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

#include "cafObservingPointer.h"

#include <deque>
#include <memory>
#include <string>

namespace caffa
{
class FieldHandle;
class ObjectHandle;
class UiEditorHandle;
class UiItem;
class UiTreeItemEditor;
class UiTreeOrdering;

//==================================================================================================
/// Class storing a tree structure representation of some Object hierarchy to be used for tree views in the Gui
//==================================================================================================

class UiTreeOrdering
{
public:
    explicit UiTreeOrdering( ObjectHandle* item );
    explicit UiTreeOrdering( FieldHandle* field );
    UiTreeOrdering( const std::string& title );

    ~UiTreeOrdering();

    UiTreeOrdering( const UiTreeOrdering& ) = delete;
    UiTreeOrdering& operator=( const UiTreeOrdering& ) = delete;

    void            add( FieldHandle* field );
    void            add( ObjectHandle* object );
    UiTreeOrdering* add( const std::string& title );

    /// If the rest of the fields containing children is supposed to be omitted, set skipRemainingFields to true.
    void skipRemainingChildren( bool doSkip = true ) { m_forgetRemainingFields = doSkip; }
    /// To stop the tree generation at this level, setIgnoreSubTree to true
    void setIgnoreSubTree( bool doIgnoreSubTree ) { m_isToIgnoreSubTree = doIgnoreSubTree; }

    // Testing for the Item being represented
    bool isRepresentingObject() const { return !isRepresentingField() && ( m_object != nullptr ); }
    bool isRepresentingField() const { return ( m_object != nullptr ) && ( m_field != nullptr ); }
    bool isDisplayItemOnly() const { return ( m_uiItem != nullptr ); }
    bool isValid() const { return ( this->activeItem() != nullptr ); }

    // Access to the Item being represented
    UiItem*       activeItem() const;
    ObjectHandle* object() const;
    FieldHandle*  field() const;
    UiItem*       uiItem() const;

    // Tree structure traversal access
    UiTreeOrdering* child( int index ) const;
    size_t          childCount() const;
    UiTreeOrdering* parent() const;
    size_t          indexInParent() const;

    // Debug helper
    void debugDump( int level ) const;

private:
    friend class ObjectHandle;
    friend class ObjectUiCapability;
    bool isIncludingRemainingChildren() const { return !m_forgetRemainingFields; }
    bool ignoreSubTree() const { return m_isToIgnoreSubTree; }
    bool containsField( const FieldHandle* field );
    bool containsObject( const ObjectHandle* object );
    void appendChild( UiTreeOrdering* child );

    friend class UiTreeViewQModel;
    UiEditorHandle* editor();
    void            setEditor( std::unique_ptr<UiEditorHandle> editor );
    void            insertChild( int position, UiTreeOrdering* child );
    bool            removeChildren( int position, int count );
    bool            removeChildrenNoDelete( int position, int count );

private:
    UiTreeOrdering( UiTreeOrdering* parent, ObjectHandle* item );
    UiTreeOrdering( UiTreeOrdering* parent, FieldHandle* field );

    // Item that we represent
    ObservingPointer<ObjectHandle>   m_object; // We keep both pointer to object and pointer to field when representing a field
    FieldHandle*            m_field; // because m_object is guarded while FieldHandle::ownerObject() is not guarded
    std::unique_ptr<UiItem> m_uiItem;

    // Tree generation control
    bool m_forgetRemainingFields;
    bool m_isToIgnoreSubTree;

    // Editor propagating changes from Item to TreeViewEditor
    std::unique_ptr<UiEditorHandle> m_treeItemEditor;

    // Tree data
    std::deque<UiTreeOrdering*> m_childItems;
    UiTreeOrdering*             m_parentItem;
};

} // End of namespace caffa
