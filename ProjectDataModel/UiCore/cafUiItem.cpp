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

#include "cafUiItem.h"
#include "cafUiEditorHandle.h"
#include "cafUiObjectEditorHandle.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItemInfo::UiItemInfo( const std::string&            uiName,
                        std::shared_ptr<IconProvider> iconProvider,
                        std::string                   toolTip /*= ""*/,
                        std::string                   whatsThis /*= ""*/,
                        std::string                   extraDebugText /*= ""*/ )
    : m_uiName( uiName )
    , m_iconProvider( iconProvider )
    , m_toolTip( toolTip )
    , m_whatsThis( whatsThis )
    , m_extraDebugText( extraDebugText )
    , m_editorTypeName( "" )
    , m_isHidden( false )
    , m_isTreeChildrenHidden( false )
    , m_isReadOnly( false )
    , m_labelAlignment( LEFT )
    , m_isCustomContextMenuEnabled( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItemInfo::UiItemInfo( const std::string& uiName,
                        std::string        iconResourceLocation /*= ""*/,
                        std::string        toolTip /*= ""*/,
                        std::string        whatsThis /*= ""*/,
                        std::string        extraDebugText /*= ""*/ )
    : m_uiName( uiName )
    , m_iconProvider( std::make_shared<IconProvider>( iconResourceLocation ) )
    , m_toolTip( toolTip )
    , m_whatsThis( whatsThis )
    , m_extraDebugText( extraDebugText )
    , m_editorTypeName( "" )
    , m_isHidden( false )
    , m_isTreeChildrenHidden( false )
    , m_isReadOnly( false )
    , m_labelAlignment( LEFT )
    , m_isCustomContextMenuEnabled( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider* UiItemInfo::iconProvider() const
{
    return m_iconProvider.get();
}

//==================================================================================================
/// UiItem
//==================================================================================================

bool UiItem::sm_showExtraDebugText = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string UiItem::uiName() const
{
    return m_itemInfo.m_uiName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiName( const std::string& uiName )
{
    m_itemInfo.m_uiName = uiName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider* UiItem::uiIconProvider() const
{
    return m_itemInfo.m_iconProvider.get();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiIcon( std::shared_ptr<IconProvider> uiIconProvider )
{
    m_itemInfo.m_iconProvider = uiIconProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiIconFromResourceString( const std::string& uiIconResourceName )
{
    setUiIcon( std::make_shared<IconProvider>( uiIconResourceName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const Color UiItem::uiContentTextColor() const
{
    return m_itemInfo.m_contentTextColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiContentTextColor( const Color& color )
{
    m_itemInfo.m_contentTextColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string UiItem::uiToolTip() const
{
    std::string text;

    if ( !m_itemInfo.m_toolTip.empty() )
    {
        text = m_itemInfo.m_toolTip;
        if ( UiItem::showExtraDebugText() && !m_itemInfo.m_extraDebugText.empty() )
        {
            text += "(" + m_itemInfo.m_extraDebugText + ")";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiToolTip( const std::string& uiToolTip )
{
    m_itemInfo.m_toolTip = uiToolTip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string UiItem::uiWhatsThis() const
{
    return m_itemInfo.m_whatsThis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiWhatsThis( const std::string& uiWhatsThis )
{
    m_itemInfo.m_whatsThis = uiWhatsThis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiHidden() const
{
    return m_itemInfo.m_isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiHidden( bool isHidden )
{
    m_itemInfo.m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiTreeHidden() const
{
    // TODO: Must be separated from uiHidden when childField object embedding is implemented
    return isUiHidden();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiTreeHidden( bool isHidden )
{
    setUiHidden( isHidden );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiTreeChildrenHidden() const
{
    return m_itemInfo.m_isTreeChildrenHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiTreeChildrenHidden( bool isTreeChildrenHidden )
{
    m_itemInfo.m_isTreeChildrenHidden = isTreeChildrenHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiReadOnly( /*= ""*/ ) const
{
    return m_itemInfo.m_isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiReadOnly( bool isReadOnly )
{
    m_itemInfo.m_isReadOnly = isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string UiItem::uiEditorTypeName() const
{
    return m_itemInfo.m_editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiEditorTypeName( const std::string& editorTypeName )
{
    m_itemInfo.m_editorTypeName = editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiGroup() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItemInfo::LabelPosType UiItem::uiLabelPosition() const
{
    return m_itemInfo.m_labelAlignment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiLabelPosition( UiItemInfo::LabelPosType alignment )
{
    m_itemInfo.m_labelAlignment = alignment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isCustomContextMenuEnabled( /*= ""*/ ) const
{
    return m_itemInfo.m_isCustomContextMenuEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setCustomContextMenuEnabled( bool enableCustomContextMenu )
{
    m_itemInfo.m_isCustomContextMenuEnabled = enableCustomContextMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const UiItemInfo& UiItem::configInfo() const
{
    return m_itemInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::updateConnectedEditors() const
{
    std::set<UiEditorHandle*>::iterator it;
    for ( it = m_editors.begin(); it != m_editors.end(); ++it )
    {
        ( *it )->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::updateAllRequiredEditors() const
{
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem::~UiItem()
{
    std::set<UiEditorHandle*>::iterator it;
    for ( it = m_editors.begin(); it != m_editors.end(); ++it )
    {
        ( *it )->m_item = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem::UiItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::updateUiIconFromState( bool isActive )
{
    if ( m_itemInfo.m_iconProvider )
    {
        m_itemInfo.m_iconProvider->setActive( isActive );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<UiEditorHandle*> UiItem::connectedEditors() const
{
    std::vector<UiEditorHandle*> editors;
    for ( auto e : m_editors )
    {
        editors.push_back( e );
    }

    return editors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::hasEditor( UiEditorHandle* editor ) const
{
    return m_editors.count( editor ) > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::showExtraDebugText()
{
    return sm_showExtraDebugText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::enableExtraDebugText( bool enable )
{
    sm_showExtraDebugText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiItemInfo( const UiItemInfo& itemInfo )
{
    m_itemInfo = itemInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::removeFieldEditor( UiEditorHandle* fieldView )
{
    m_editors.erase( fieldView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::addFieldEditor( UiEditorHandle* fieldView )
{
    m_editors.insert( fieldView );
}

} // End of namespace caffa
