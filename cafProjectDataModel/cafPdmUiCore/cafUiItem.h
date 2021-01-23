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

#include "cafColor.h"
#include "cafIconProvider.h"
#include "cafOptionItemInfo.h"
#include "cafUiFieldSpecialization.h"
#include "cafVariant.h"

#include <deque>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Finds the indexes into the optionList that the field value(s) corresponds to.
/// In the case where the field is some kind of array, several indexes might be returned
/// The returned bool is true if all the fieldValues were found.
//--------------------------------------------------------------------------------------------------
template <typename T>
bool findOptionItemValues( const std::deque<OptionItemInfo>& optionList, Variant fieldValue, std::vector<int>& foundIndexes )
{
    foundIndexes.clear();

    // Find this fieldvalue in the optionlist if present

    // First handle lists/arrays of values
    if ( fieldValue.isVector() )
    {
        std::vector<Variant> valuesSelectedInField = fieldValue.toVector();

        if ( valuesSelectedInField.size() )
        {
            // Create a list to be able to remove items as they are matched with values
            std::vector<std::pair<Variant, int>> optionVariantAndIndexPairs;

            for ( int i = 0; i < optionList.size(); ++i )
            {
                Variant optionVariant( optionList[i].value() );
                optionVariantAndIndexPairs.push_back( std::make_pair( optionVariant, i ) );
            }

            for ( int i = 0; i < valuesSelectedInField.size(); ++i )
            {
                for ( auto it = optionVariantAndIndexPairs.begin(); it != optionVariantAndIndexPairs.end(); ++it )
                {
                    if ( UiFieldSpecialization<T>::isDataElementEqual( valuesSelectedInField[i], it->first ) )
                    {
                        foundIndexes.push_back( it->second );

                        // Assuming that one option is referenced only once, the option is erased. Then break
                        // out of the inner loop, as this operation can be costly for fields with many options and many
                        // values

                        optionVariantAndIndexPairs.erase( it );
                        break;
                    }
                }
            }
        }

        return ( static_cast<size_t>( valuesSelectedInField.size() ) <= foundIndexes.size() );
    }
    else // Then handle single value fields
    {
        for ( int opIdx = 0; opIdx < static_cast<int>( optionList.size() ); ++opIdx )
        {
            if ( UiFieldSpecialization<T>::isDataElementEqual( optionList[opIdx].value(), fieldValue ) )
            {
                foundIndexes.push_back( opIdx );
                break;
            }
        }
        return ( foundIndexes.size() > (size_t)0 );
    }
}


//==================================================================================================
/// Class to keep (principally static) gui presentation information
/// of a data structure item (field or object) used by UiItem
//==================================================================================================

class UiItemInfo
{
public:
    enum LabelPosType
    {
        LEFT,
        TOP,
        HIDDEN
    };

    UiItemInfo()
        : m_editorTypeName( "" )
        , m_isHidden( -1 )
        , m_isTreeChildrenHidden( -1 )
        , m_isReadOnly( -1 )
        , m_labelAlignment( LEFT )
        , m_isCustomContextMenuEnabled( -1 )
    {
    }

    UiItemInfo( const std::string& uiName,
                std::string        iconResourceLocation = "",
                std::string        toolTip              = "",
                std::string        whatsThis            = "",
                std::string        extraDebugText       = "" );

    UiItemInfo( const std::string&            uiName,
                std::shared_ptr<IconProvider> iconProvider,
                std::string                   toolTip        = "",
                std::string                   whatsThis      = "",
                std::string                   extraDebugText = "" );

    const IconProvider* iconProvider() const;

private:
    friend class UiItem;
    std::string                   m_uiName;
    std::shared_ptr<IconProvider> m_iconProvider;
    Color       m_contentTextColor; ///< Color of a fields value text. Invalid by default. An Invalid color is not used.
    std::string m_toolTip;
    std::string m_whatsThis;
    std::string m_extraDebugText;
    std::string m_editorTypeName; ///< Use this exact type of editor to edit this UiItem
    std::string m_3dEditorTypeName; ///< If set, use this editor type to edit this UiItem in 3D
    int         m_isHidden; ///< UiItem should be hidden. -1 means not set
    int         m_isTreeChildrenHidden; ///< Children of UiItem should be hidden. -1 means not set
    int         m_isReadOnly; ///< UiItem should be insensitive, or read only. -1 means not set.
    LabelPosType m_labelAlignment;
    int          m_isCustomContextMenuEnabled;
};

class UiEditorHandle;


//==================================================================================================
/// Base class for all data structure items (fields or objects) to make them have information on
/// how to display them in the GUI.
//==================================================================================================

class UiItem
{
public:
    UiItem();
    virtual ~UiItem();

    UiItem( const UiItem& ) = delete;
    UiItem& operator=( const UiItem& ) = delete;

    const std::string uiName() const;
    void              setUiName( const std::string& = "" );

    const IconProvider* uiIconProvider() const;
    void                setUiIcon( std::shared_ptr<IconProvider> uiIcon );
    void                setUiIconFromResourceString( const std::string& uiIconResourceName );

    const Color uiContentTextColor() const;
    void        setUiContentTextColor( const Color& uiIcon );

    const std::string uiToolTip() const;
    void              setUiToolTip( const std::string& uiToolTip );

    const std::string uiWhatsThis() const;
    void              setUiWhatsThis( const std::string& uiWhatsThis );

    bool isUiHidden() const;
    void setUiHidden( bool isHidden );

    bool isUiTreeHidden() const;
    void setUiTreeHidden( bool isHidden );

    bool isUiTreeChildrenHidden() const;
    void setUiTreeChildrenHidden( bool isTreeChildrenHidden );

    bool isUiReadOnly() const;
    void setUiReadOnly( bool isReadOnly );

    UiItemInfo::LabelPosType uiLabelPosition() const;
    void                     setUiLabelPosition( UiItemInfo::LabelPosType alignment );

    bool isCustomContextMenuEnabled() const;
    void setCustomContextMenuEnabled( bool enableCustomContextMenu );

    std::string uiEditorTypeName() const;
    void        setUiEditorTypeName( const std::string& editorTypeName );

    virtual bool isUiGroup() const;

    /// Intended to be called when fields in an object has been changed
    void updateConnectedEditors() const;

    /// Intended to be called when an object has been created or deleted
    void updateAllRequiredEditors() const;

    void updateUiIconFromState( bool isActive );

    std::vector<UiEditorHandle*> connectedEditors() const;

    bool hasEditor( UiEditorHandle* editor ) const;

    static bool showExtraDebugText();
    static void enableExtraDebugText( bool enable );

public: // Pdm-Private only
    //==================================================================================================
    /// This method sets the GUI description pointer, which is supposed to be statically allocated
    /// somewhere. the PdmGuiEntry class will not delete it in any way, and always trust it to be present.
    /// Consider as PRIVATE to the PdmSystem
    //==================================================================================================

    const UiItemInfo& configInfo() const;
    void              setUiItemInfo( const UiItemInfo& itemInfo );

    void removeFieldEditor( UiEditorHandle* fieldView );
    void addFieldEditor( UiEditorHandle* fieldView );

protected:
    std::set<UiEditorHandle*> m_editors;

private:
    UiItemInfo m_itemInfo;

    static bool sm_showExtraDebugText;
};

} // End of namespace caf
