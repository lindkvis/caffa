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

#include "cafUiItem.h"

#include <string>
#include <vector>

namespace caf
{
class UiGroup;
class FieldHandle;
class ObjectHandle;

//==================================================================================================
/// Class storing the order and grouping of fields and groups of fields etc. to be used in the Gui
//==================================================================================================

class UiOrdering
{
public:
    struct LayoutOptions
    {
        static const int MAX_COLUMN_SPAN = -1;
        LayoutOptions( bool newRow = true, int totalColumnSpan = MAX_COLUMN_SPAN, int leftLabelColumnSpan = MAX_COLUMN_SPAN )
            : newRow( newRow )
            , totalColumnSpan( totalColumnSpan )
            , leftLabelColumnSpan( leftLabelColumnSpan )
        {
        }

        bool newRow;
        int  totalColumnSpan;
        int  leftLabelColumnSpan;
    };
    typedef std::pair<UiItem*, LayoutOptions> FieldAndLayout;
    typedef std::vector<FieldAndLayout>       RowLayout;
    typedef std::vector<RowLayout>            TableLayout;

    UiOrdering()
        : m_skipRemainingFields( false ){};
    virtual ~UiOrdering();

    UiOrdering( const UiOrdering& ) = delete;
    UiOrdering& operator=( const UiOrdering& ) = delete;

    void add( const FieldHandle* field, LayoutOptions layout = LayoutOptions() );
    void add( const ObjectHandle* obj, LayoutOptions layout = LayoutOptions() );
    bool insertBeforeGroup( const std::string& groupId,
                            const FieldHandle* fieldToInsert,
                            LayoutOptions      layout = LayoutOptions() );
    bool insertBeforeItem( const UiItem* item, const FieldHandle* fieldToInsert, LayoutOptions layout = LayoutOptions() );

    UiGroup* addNewGroup( const std::string& displayName, LayoutOptions layout = LayoutOptions() );
    UiGroup* createGroupBeforeGroup( const std::string& groupId,
                                     const std::string& displayName,
                                     LayoutOptions      layout = LayoutOptions() );
    UiGroup* createGroupBeforeItem( const UiItem*      item,
                                    const std::string& displayName,
                                    LayoutOptions      layout = LayoutOptions() );

    UiGroup* addNewGroupWithKeyword( const std::string& displayName,
                                     const std::string& groupKeyword,
                                     LayoutOptions      layout = LayoutOptions() );
    UiGroup* createGroupWithIdBeforeGroup( const std::string& groupId,
                                           const std::string& displayName,
                                           const std::string& newGroupId,
                                           LayoutOptions      layout = LayoutOptions() );
    UiGroup* createGroupWithIdBeforeItem( const UiItem*      item,
                                          const std::string& displayName,
                                          const std::string& newGroupId,
                                          LayoutOptions      layout = LayoutOptions() );

    UiGroup* findGroup( const std::string& groupId ) const;

    void skipRemainingFields( bool doSkip = true );

    // Pdm internal methods

    const std::vector<UiItem*>         uiItems() const;
    const std::vector<FieldAndLayout>& uiItemsWithLayout() const;

    std::vector<std::vector<FieldAndLayout>> calculateTableLayout() const;
    int                                      nrOfColumns( const TableLayout& tableLayout ) const;
    int                                      nrOfRequiredColumnsInRow( const RowLayout& rowItems ) const;
    int                                      nrOfExpandingItemsInRow( const RowLayout& rowItems ) const;
    void                                     nrOfColumnsRequiredForItem( const FieldAndLayout& fieldAndLayout,
                                                                         int*                  totalColumnsRequired,
                                                                         int*                  labelColumnsRequired,
                                                                         int*                  fieldColumnsRequired ) const;
    bool                                     contains( const UiItem* item ) const;
    bool                                     isIncludingRemainingFields() const;

protected:
    struct PositionFound
    {
        UiOrdering* parent;
        size_t      indexInParent;
        UiItem*     item();
        UiGroup*    group();
    };

    PositionFound findGroupPosition( const std::string& groupKeyword ) const;
    PositionFound findItemPosition( const UiItem* item ) const;

private:
    void     insert( size_t index, const FieldHandle* field, LayoutOptions layout = LayoutOptions() );
    UiGroup* insertNewGroupWithKeyword( size_t             index,
                                        const std::string& displayName,
                                        const std::string& groupKeyword,
                                        LayoutOptions      layout = LayoutOptions() );

    std::vector<FieldAndLayout> m_ordering; ///< The order of groups and fields
    std::vector<UiGroup*>       m_createdGroups; ///< Owned UiGroups, for memory management only
    bool                        m_skipRemainingFields;
};

} // End of namespace caf

#include "cafUiGroup.h"
