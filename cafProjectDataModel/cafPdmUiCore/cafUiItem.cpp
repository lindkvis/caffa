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
#include "cafPtrField.h"
#include "cafUiEditorHandle.h"
#include "cafUiObjectEditorHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItemInfo::UiItemInfo( const QString& uiName,
                              IconProvider   iconProvider /*= IconProvider() */,
                              QString        toolTip /*= ""*/,
                              QString        whatsThis /*= ""*/,
                              QString        extraDebugText /*= ""*/ )
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
UiItemInfo::UiItemInfo( const QString& uiName,
                              QString        iconResourceLocation /*= ""*/,
                              QString        toolTip /*= ""*/,
                              QString        whatsThis /*= ""*/,
                              QString        extraDebugText /*= ""*/ )
    : m_uiName( uiName )
    , m_iconProvider( iconResourceLocation )
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
std::unique_ptr<QIcon> UiItemInfo::icon() const
{
    return m_iconProvider.icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider& UiItemInfo::iconProvider() const
{
    return m_iconProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo( const QString&      anOptionUiText,
                                      const QVariant&     aValue,
                                      bool                isReadOnly /* = false */,
                                      const IconProvider& anIcon /* = IconProvider()*/ )
    : m_optionUiText( anOptionUiText )
    , m_value( aValue )
    , m_isReadOnly( isReadOnly )
    , m_iconProvider( anIcon )
    , m_level( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo( const QString&        anOptionUiText,
                                      caf::ObjectHandle* obj,
                                      bool                  isReadOnly /*= false*/,
                                      const IconProvider&   anIcon /*= IconProvider()*/ )
    : m_optionUiText( anOptionUiText )
    , m_isReadOnly( isReadOnly )
    , m_iconProvider( anIcon )
    , m_level( 0 )
{
    m_value = QVariant::fromValue( caf::PdmPointer<caf::ObjectHandle>( obj ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo PdmOptionItemInfo::createHeader( const QString&      anOptionUiText,
                                                   bool                isReadOnly /*= false*/,
                                                   const IconProvider& anIcon /*= IconProvider()*/ )
{
    PdmOptionItemInfo header( anOptionUiText, QVariant(), isReadOnly, anIcon );

    return header;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmOptionItemInfo::setLevel( int level )
{
    m_level = level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmOptionItemInfo::optionUiText() const
{
    return m_optionUiText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QVariant PdmOptionItemInfo::value() const
{
    return m_value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isReadOnly() const
{
    return m_isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isHeading() const
{
    return !m_value.isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> PdmOptionItemInfo::icon() const
{
    return m_iconProvider.icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmOptionItemInfo::level() const
{
    return m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList PdmOptionItemInfo::extractUiTexts( const QList<PdmOptionItemInfo>& optionList )
{
    QStringList texts;

    for ( const auto& option : optionList )
    {
        texts.push_back( option.optionUiText() );
    }

    return texts;
}

//==================================================================================================
/// UiItem
//==================================================================================================

bool UiItem::sm_showExtraDebugText = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString UiItem::uiName( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_uiName.isNull() ) ) return conInfo->m_uiName;
    if ( defInfo && !( defInfo->m_uiName.isNull() ) ) return defInfo->m_uiName;
    if ( sttInfo && !( sttInfo->m_uiName.isNull() ) ) return sttInfo->m_uiName;

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiName( const QString& uiName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_uiName = uiName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> UiItem::uiIcon( const QString& uiConfigName ) const
{
    return uiIconProvider( uiConfigName ).icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider UiItem::uiIconProvider( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->iconProvider().valid() ) return conInfo->iconProvider();
    if ( defInfo && defInfo->iconProvider().valid() ) return defInfo->iconProvider();
    if ( sttInfo && sttInfo->iconProvider().valid() ) return sttInfo->iconProvider();

    return IconProvider();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiIcon( const IconProvider& uiIconProvider, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_iconProvider = uiIconProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiIconFromResourceString( const QString& uiIconResourceName, const QString& uiConfigName /*= ""*/ )
{
    setUiIcon( caf::IconProvider( uiIconResourceName ), uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QColor UiItem::uiContentTextColor( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && ( conInfo->m_contentTextColor.isValid() ) ) return conInfo->m_contentTextColor;
    if ( defInfo && ( defInfo->m_contentTextColor.isValid() ) ) return defInfo->m_contentTextColor;
    if ( sttInfo && ( sttInfo->m_contentTextColor.isValid() ) ) return sttInfo->m_contentTextColor;

    return QColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiContentTextColor( const QColor& color, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_contentTextColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString UiItem::uiToolTip( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    QString text;

    if ( conInfo && !( conInfo->m_toolTip.isNull() ) )
    {
        text = conInfo->m_toolTip;
        if ( UiItem::showExtraDebugText() && !conInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( conInfo->m_extraDebugText );
        }
    }

    if ( text.isEmpty() && defInfo && !( defInfo->m_toolTip.isNull() ) )
    {
        text = defInfo->m_toolTip;
        if ( UiItem::showExtraDebugText() && !defInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( defInfo->m_extraDebugText );
        }
    }

    if ( text.isEmpty() && sttInfo && !( sttInfo->m_toolTip.isNull() ) )
    {
        text = sttInfo->m_toolTip;
        if ( UiItem::showExtraDebugText() && !sttInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( sttInfo->m_extraDebugText );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiToolTip( const QString& uiToolTip, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_toolTip = uiToolTip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString UiItem::uiWhatsThis( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_whatsThis.isNull() ) ) return conInfo->m_whatsThis;
    if ( defInfo && !( defInfo->m_whatsThis.isNull() ) ) return defInfo->m_whatsThis;
    if ( sttInfo && !( sttInfo->m_whatsThis.isNull() ) ) return sttInfo->m_whatsThis;

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiWhatsThis( const QString& uiWhatsThis, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_whatsThis = uiWhatsThis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiHidden( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_isHidden == -1 ) ) return conInfo->m_isHidden;
    if ( defInfo && !( defInfo->m_isHidden == -1 ) ) return defInfo->m_isHidden;
    if ( sttInfo && !( sttInfo->m_isHidden == -1 ) ) return sttInfo->m_isHidden;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiHidden( bool isHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiTreeHidden( const QString& uiConfigName ) const
{
    // TODO: Must be separated from uiHidden when childField object embedding is implemented

    return isUiHidden( uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiTreeHidden( bool isHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiTreeChildrenHidden( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_isTreeChildrenHidden == -1 ) ) return conInfo->m_isTreeChildrenHidden;
    if ( defInfo && !( defInfo->m_isTreeChildrenHidden == -1 ) ) return defInfo->m_isTreeChildrenHidden;
    if ( sttInfo && !( sttInfo->m_isTreeChildrenHidden == -1 ) ) return sttInfo->m_isTreeChildrenHidden;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiTreeChildrenHidden( bool isTreeChildrenHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isTreeChildrenHidden = isTreeChildrenHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isUiReadOnly( const QString& uiConfigName /*= ""*/ ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_isReadOnly == -1 ) ) return conInfo->m_isReadOnly;
    if ( defInfo && !( defInfo->m_isReadOnly == -1 ) ) return defInfo->m_isReadOnly;
    if ( sttInfo && !( sttInfo->m_isReadOnly == -1 ) ) return sttInfo->m_isReadOnly;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiReadOnly( bool isReadOnly, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isReadOnly = isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString UiItem::uiEditorTypeName( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_editorTypeName.isEmpty() ) ) return conInfo->m_editorTypeName;
    if ( defInfo && !( defInfo->m_editorTypeName.isEmpty() ) ) return defInfo->m_editorTypeName;
    if ( sttInfo && !( sttInfo->m_editorTypeName.isEmpty() ) ) return sttInfo->m_editorTypeName;

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiEditorTypeName( const QString& editorTypeName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_editorTypeName = editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString UiItem::ui3dEditorTypeName( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_3dEditorTypeName.isEmpty() ) ) return conInfo->m_3dEditorTypeName;
    if ( defInfo && !( defInfo->m_3dEditorTypeName.isEmpty() ) ) return defInfo->m_3dEditorTypeName;
    if ( sttInfo && !( sttInfo->m_3dEditorTypeName.isEmpty() ) ) return sttInfo->m_3dEditorTypeName;

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUi3dEditorTypeName( const QString& editorTypeName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_3dEditorTypeName = editorTypeName;
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
UiItemInfo::LabelPosType UiItem::uiLabelPosition( const QString& uiConfigName ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo ) return conInfo->m_labelAlignment;
    if ( defInfo ) return defInfo->m_labelAlignment;
    if ( sttInfo ) return sttInfo->m_labelAlignment;

    return UiItemInfo::LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setUiLabelPosition( UiItemInfo::LabelPosType alignment, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_labelAlignment = alignment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiItem::isCustomContextMenuEnabled( const QString& uiConfigName /*= ""*/ ) const
{
    const UiItemInfo* conInfo = configInfo( uiConfigName );
    const UiItemInfo* defInfo = defaultInfo();
    const UiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && ( conInfo->m_isCustomContextMenuEnabled != -1 ) ) return conInfo->m_isCustomContextMenuEnabled;
    if ( defInfo && ( defInfo->m_isCustomContextMenuEnabled != -1 ) ) return defInfo->m_isCustomContextMenuEnabled;
    if ( sttInfo && ( sttInfo->m_isCustomContextMenuEnabled != -1 ) ) return sttInfo->m_isCustomContextMenuEnabled;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::setCustomContextMenuEnabled( bool enableCustomContextMenu, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isCustomContextMenuEnabled = enableCustomContextMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const UiItemInfo* UiItem::configInfo( const QString& uiConfigName ) const
{
    if ( uiConfigName == "" || uiConfigName.isNull() ) return nullptr;

    std::map<QString, UiItemInfo>::const_iterator it;
    it = m_configItemInfos.find( uiConfigName );

    if ( it != m_configItemInfos.end() ) return &( it->second );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const UiItemInfo* UiItem::defaultInfo() const
{
    std::map<QString, UiItemInfo>::const_iterator it;
    it = m_configItemInfos.find( "" );

    if ( it != m_configItemInfos.end() ) return &( it->second );

    return nullptr;
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

    PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem::~UiItem()
{
    std::set<UiEditorHandle*>::iterator it;
    for ( it = m_editors.begin(); it != m_editors.end(); ++it )
    {
        ( *it )->m_pdmItem = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem::UiItem()
    : m_staticItemInfo( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiItem::updateUiIconFromState( bool isActive, const QString& uiConfigName )
{
    IconProvider normalIconProvider = this->uiIconProvider( uiConfigName );
    normalIconProvider.setActive( isActive );
    this->setUiIcon( normalIconProvider, uiConfigName );
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
void UiItem::setUiItemInfo( UiItemInfo* itemInfo )
{
    m_staticItemInfo = itemInfo;
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

} // End of namespace caf
