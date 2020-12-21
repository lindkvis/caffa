//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafUiTableViewQModel.h"

#include "cafChildArrayField.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafQVariantConverter.h"
#include "cafSelectionManager.h"
#include "cafUiComboBoxEditor.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiDateEditor.h"
#include "cafUiFieldEditorHelper.h"
#include "cafUiLineEditor.h"
#include "cafUiTableRowEditor.h"
#include "cafUiTableView.h"

#include <QTableView>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableViewQModel::PdmUiTableViewQModel( QWidget* parent )
    : QAbstractTableModel( parent )
{
    m_pdmList = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::rowCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    auto childArrayField = childArrayFieldHandle();
    if ( childArrayField )
    {
        size_t itemCount = childArrayField->size();
        return static_cast<int>( itemCount );
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::columnCount( const QModelIndex& parent /*= QModelIndex( ) */ ) const
{
    return static_cast<int>( m_modelColumnIndexToFieldIndex.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTableViewQModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    if ( role == Qt::DisplayRole )
    {
        if ( orientation == Qt::Horizontal )
        {
            FieldUiCapability* uiFieldHandle = getUiFieldHandle( createIndex( 0, section ) );
            if ( uiFieldHandle )
            {
                return QVariant::fromValue( QString::fromStdString( uiFieldHandle->uiName() ) );
            }
        }
        else if ( orientation == Qt::Vertical )
        {
            return section + 1;
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags PdmUiTableViewQModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() ) return Qt::ItemIsEnabled;

    Qt::ItemFlags flagMask = QAbstractItemModel::flags( index );

    if ( isRepresentingBoolean( index ) )
    {
        flagMask = flagMask | Qt::ItemIsUserCheckable;
    }
    else
    {
        flagMask = flagMask | Qt::ItemIsEditable;
    }

    FieldUiCapability* uiFieldHandle = getUiFieldHandle( index );
    if ( uiFieldHandle )
    {
        if ( uiFieldHandle->isUiReadOnly() )
        {
            if ( flagMask & Qt::ItemIsUserCheckable )
            {
                flagMask = flagMask & ( ~Qt::ItemIsEnabled );
            }
            else
            {
                flagMask = flagMask ^ Qt::ItemIsEditable; // To make it selectable, but not editable
            }
        }
    }
    return flagMask;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewQModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
    if ( role == Qt::CheckStateRole )
    {
        if ( isRepresentingBoolean( index ) )
        {
            bool toggleOn = ( value == Qt::Checked );

            FieldUiCapability* uiFieldHandle = getUiFieldHandle( index );
            if ( uiFieldHandle )
            {
                UiCommandSystemProxy::instance()->setUiValueToField( uiFieldHandle, toggleOn );

                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTableViewQModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole */ ) const
{
    if ( role == Qt::ForegroundRole )
    {
        FieldHandle* fieldHandle = getField( index );
        if ( fieldHandle && fieldHandle->capability<FieldUiCapability>() )
        {
            Color  storedColor = fieldHandle->capability<FieldUiCapability>()->uiContentTextColor();
            QColor textColor   = storedColor.to<QColor>();

            if ( fieldHandle->capability<FieldUiCapability>()->isUiReadOnly() )
            {
                if ( textColor.isValid() )
                {
                    return textColor.lighter( 150 );
                }
                else
                {
                    return QColor( Qt::lightGray );
                }
            }
            else if ( textColor.isValid() )
            {
                return textColor;
            }
        }
    }

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        FieldHandle* fieldHandle = getField( index );

        FieldUiCapability* uiFieldHandle = fieldHandle->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            Variant fieldValue = uiFieldHandle->uiValue();

            std::deque<OptionItemInfo> options;
            bool                       useOptionsOnly = true;

            options = uiFieldHandle->valueOptions( &useOptionsOnly );
            CAF_ASSERT( useOptionsOnly ); // Not supported

            if ( fieldValue.isVector() )
            {
                QString displayText;

                std::vector<Variant> valuesSelectedInField = fieldValue.toVector();

                if ( !valuesSelectedInField.empty() )
                {
                    for ( const Variant& v : valuesSelectedInField )
                    {
                        auto opit = std::find_if( options.begin(), options.end(), [&v]( const auto& option ) {
                            return option.value() == v;
                        } );
                        if ( opit != options.end() )
                        {
                            if ( !displayText.isEmpty() ) displayText += ", ";

                            displayText += QString::fromStdString( opit->optionUiText() );
                        }
                    }
                }

                return displayText;
            }

            if ( !options.empty() )
            {
                QString displayText;
                auto    opit = std::find_if( options.begin(), options.end(), [&fieldValue]( const auto& option ) {
                    return option.value() == fieldValue;
                } );

                if ( opit != options.end() )
                {
                    displayText = QString::fromStdString( opit->optionUiText() );
                }

                return displayText;
            }

            QVariant val;

            ObjectHandle*       objForRow   = this->pdmObjectForRow( index.row() );
            ObjectUiCapability* uiObjForRow = uiObj( objForRow );
            if ( uiObjForRow )
            {
                // NOTE: Redesign
                // To be able to get formatted string, an editor attribute concept is used
                // TODO: Create a function in pdmObject like this
                // virtual void            defineDisplayString(const FieldHandle* field, QString ) {}

                {
                    PdmUiLineEditorAttributeUiDisplayString leab;
                    uiObjForRow->editorAttribute( fieldHandle, &leab );

                    if ( !leab.m_displayString.empty() )
                    {
                        val = QVariant( QString::fromStdString( leab.m_displayString ) );
                    }
                }

                if ( val.isNull() )
                {
                    PdmUiDateEditorAttribute leab;
                    uiObjForRow->editorAttribute( fieldHandle, &leab );

                    auto dateFormat = leab.dateFormat;
                    if ( !dateFormat.empty() )
                    {
                        std::time_t time     = uiFieldHandle->uiValue().value<std::time_t>();
                        QDateTime   dateTime = QDateTime::fromTime_t( time );
                        QDate       date     = dateTime.date();
                        if ( date.isValid() )
                        {
                            QString displayString = date.toString( QString::fromStdString( dateFormat ) );
                            val                   = displayString;
                        }
                    }
                }
                if ( val.isNull() )
                {
                    val = QString::fromStdString( uiFieldHandle->uiValue().value<std::string>( "" ) );
                }
            }
            else
            {
                val = QString::fromStdString( uiFieldHandle->uiValue().value<std::string>( "" ) );
            }

            return val;
        }
        else
        {
            CAF_ASSERT( false );
        }
    }
    else if ( role == Qt::CheckStateRole )
    {
        if ( isRepresentingBoolean( index ) )
        {
            FieldUiCapability* uiFieldHandle = getField( index )->capability<FieldUiCapability>();
            if ( uiFieldHandle )
            {
                Variant val         = uiFieldHandle->uiValue();
                bool    isToggledOn = val.value<bool>();
                if ( isToggledOn )
                {
                    return Qt::Checked;
                }
                else
                {
                    return Qt::Unchecked;
                }
            }
            else
            {
                return QVariant();
            }
        }
    }
    else if ( role == Qt::ToolTipRole )
    {
        FieldUiCapability* uiFieldHandle = getField( index )->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            return QString::fromStdString( uiFieldHandle->uiToolTip() );
        }
        else
        {
            return QVariant();
        }
    }
    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::setArrayFieldAndBuildEditors( ChildArrayFieldHandle* listField )
{
    beginResetModel();

    {
        ObjectHandle* ownerObject = nullptr;
        if ( listField )
        {
            ownerObject = listField->ownerObject();
        }

        if ( ownerObject )
        {
            m_pdmList     = listField;
            m_ownerObject = ownerObject;
        }
        else
        {
            m_ownerObject = nullptr;
            m_pdmList     = nullptr;
        }
    }

    UiOrdering configForFirstObject;

    if ( m_pdmList && !m_pdmList->empty() )
    {
        ObjectHandle*       firstObject            = m_pdmList->at( 0 );
        ObjectUiCapability* uiHandleForFirstObject = firstObject->capability<ObjectUiCapability>();
        if ( uiHandleForFirstObject )
        {
            uiHandleForFirstObject->uiOrdering( configForFirstObject );
            uiHandleForFirstObject->objectEditorAttribute( &m_pushButtonEditorAttributes );
        }
    }

    const std::vector<UiItem*>& uiItems = configForFirstObject.uiItems();

    std::set<std::string> usedFieldKeywords;

    m_modelColumnIndexToFieldIndex.clear();

    for ( auto uiItem : uiItems )
    {
        if ( uiItem->isUiHidden() ) continue;

        if ( uiItem->isUiGroup() ) continue;

        {
            FieldUiCapability*   field       = dynamic_cast<FieldUiCapability*>( uiItem );
            UiFieldEditorHandle* fieldEditor = nullptr;

            // Find or create FieldEditor
            auto it = m_fieldEditors.find( field->fieldHandle()->keyword() );

            if ( it == m_fieldEditors.end() )
            {
                fieldEditor = UiFieldEditorHelper::createFieldEditorForField( field );

                if ( fieldEditor )
                {
                    fieldEditor->setUiField( field );
                    m_fieldEditors[field->fieldHandle()->keyword()] = fieldEditor;
                }
            }
            else
            {
                fieldEditor = it->second;
                fieldEditor->setUiField( field );
            }

            if ( fieldEditor )
            {
                usedFieldKeywords.insert( field->fieldHandle()->keyword() );

                // TODO: Create/update is not required at this point, as UI is recreated in
                // getEditorWidgetAndTransferOwnership()
                // Can be moved, but a move will require changes in UiFieldEditorHandle
                fieldEditor->createWidgets( nullptr );
                fieldEditor->updateUi();

                int fieldIndex = getFieldIndex( field->fieldHandle() );
                m_modelColumnIndexToFieldIndex.push_back( fieldIndex );
            }
        }
    }

    // Remove all fieldViews not mentioned by the configuration from the layout

    std::vector<std::string> fvhToRemoveFromMap;
    for ( auto it = m_fieldEditors.begin(); it != m_fieldEditors.end(); ++it )
    {
        if ( usedFieldKeywords.count( it->first ) == 0 )
        {
            UiFieldEditorHandle* fvh = it->second;
            delete fvh;
            fvhToRemoveFromMap.push_back( it->first );
        }
    }

    for ( const auto& fieldEditorName : fvhToRemoveFromMap )
    {
        m_fieldEditors.erase( fieldEditorName );
    }

    recreateTableItemEditors();

    endResetModel();

    if ( m_pdmList )
    {
        // Update UI for all cells, as the content potentially has changed
        // This will probably cause performance issues for large tables

        for ( auto tableItemEditor : m_tableRowEditors )
        {
            tableItemEditor->updateUi();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* PdmUiTableViewQModel::getField( const QModelIndex& index ) const
{
    auto childArrayField = childArrayFieldHandle();

    if ( childArrayField && index.row() < static_cast<int>( childArrayField->size() ) )
    {
        ObjectHandle* pdmObject = childArrayField->at( index.row() );
        if ( pdmObject )
        {
            std::vector<FieldHandle*> fields;
            pdmObject->fields( fields );

            int fieldIndex = m_modelColumnIndexToFieldIndex[index.column()];
            if ( fieldIndex < static_cast<int>( fields.size() ) )
            {
                return fields[fieldIndex];
            }
            else
            {
                CAF_ASSERT( false );
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiFieldEditorHandle* PdmUiTableViewQModel::getEditor( const QModelIndex& index )
{
    FieldHandle* field = getField( index );
    if ( !field )
    {
        return nullptr;
    }

    UiFieldEditorHandle* editor = nullptr;

    std::map<std::string, UiFieldEditorHandle*>::iterator it;
    it = m_fieldEditors.find( field->keyword() );

    if ( it != m_fieldEditors.end() )
    {
        editor = it->second;
        if ( editor )
        {
            if ( field )
            {
                editor->setUiField( field->capability<FieldUiCapability>() );
            }
        }
    }

    return editor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewQModel::getEditorWidgetAndTransferOwnership( QWidget* parent, const QModelIndex& index )
{
    UiFieldEditorHandle* editor = getEditor( index );
    if ( editor )
    {
        // Recreate editor widget, as the delegate takes ownership of the QWidget and destroys it when
        // edit is completed. This will cause the editor widget pointer to be NULL, as it is a guarded pointer
        // using QPointer
        editor->createWidgets( parent );
        QWidget* editorWidget = editor->editorWidget();
        editorWidget->setParent( parent );

        return editorWidget;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::notifyDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    emit dataChanged( topLeft, bottomRight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::recreateTableItemEditors()
{
    for ( PdmUiTableRowEditor* tableItemEditor : m_tableRowEditors )
    {
        delete tableItemEditor;
    }
    m_tableRowEditors.clear();

    auto childArrayField = childArrayFieldHandle();
    if ( childArrayField )
    {
        for ( size_t i = 0; i < childArrayField->size(); i++ )
        {
            ObjectHandle* pdmObject = childArrayField->at( i );
            m_tableRowEditors.push_back( new PdmUiTableRowEditor( this, pdmObject, static_cast<int>( i ) ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FieldUiCapability* PdmUiTableViewQModel::getUiFieldHandle( const QModelIndex& index ) const
{
    auto fieldHandle = getField( index );
    if ( fieldHandle )
    {
        return fieldHandle->capability<FieldUiCapability>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* PdmUiTableViewQModel::pdmObjectForRow( int row ) const
{
    auto childArrayField = childArrayFieldHandle();
    if ( childArrayField && row < static_cast<int>( childArrayField->size() ) )
    {
        return childArrayField->at( row );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewQModel::isRepresentingBoolean( const QModelIndex& index ) const
{
    FieldHandle* fieldHandle = getField( index );
    if ( fieldHandle )
    {
        if ( m_pushButtonEditorAttributes.showPushButtonForFieldKeyword( fieldHandle->keyword() ) )
        {
            return false;
        }

        Variant val = fieldHandle->capability<FieldUiCapability>()->uiValue();
        if ( val.canConvert<bool>() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewQModel::createPersistentPushButtonWidgets( QTableView* tableView )
{
    if ( rowCount() > 0 )
    {
        for ( int col = 0; col < columnCount(); col++ )
        {
            FieldHandle* fieldHandle = getField( createIndex( 0, col ) );
            if ( m_pushButtonEditorAttributes.showPushButtonForFieldKeyword( fieldHandle->keyword() ) )
            {
                for ( int row = 0; row < rowCount(); row++ )
                {
                    QModelIndex mi = createIndex( row, col );

                    tableView->setIndexWidget( mi,
                                               new TableViewPushButton( getField( mi )->capability<FieldUiCapability>(),
                                                                        QString::fromStdString(
                                                                            m_pushButtonEditorAttributes.pushButtonText(
                                                                                fieldHandle->keyword() ) ) ) );
                    tableView->openPersistentEditor( mi );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QItemSelection PdmUiTableViewQModel::modelIndexFromObject( ObjectHandle* pdmObject )
{
    QItemSelection itemSelection;

    for ( int i = 0; i < this->rowCount(); i++ )
    {
        ObjectHandle* obj = this->pdmObjectForRow( i );
        if ( obj == pdmObject )
        {
            // Select whole row
            itemSelection.select( this->createIndex( i, 0 ), this->createIndex( i, this->columnCount() ) );
        }
    }

    return itemSelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ChildArrayFieldHandle* PdmUiTableViewQModel::childArrayFieldHandle() const
{
    // Required to have a PdmPointer to the owner object. Used to guard access to a field inside this object. It is not
    // possible to use a PdmPointer on a field pointer
    if ( m_ownerObject.isNull() )
    {
        return nullptr;
    }

    return m_pdmList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUiTableViewQModel::getFieldIndex( FieldHandle* field ) const
{
    auto childArrayField = childArrayFieldHandle();

    if ( childArrayField && !childArrayField->empty() )
    {
        ObjectHandle* pdmObject = childArrayField->at( 0 );
        if ( pdmObject )
        {
            std::vector<FieldHandle*> fields;
            pdmObject->fields( fields );

            for ( size_t i = 0; i < fields.size(); i++ )
            {
                if ( fields[i]->keyword() == field->keyword() )
                {
                    return static_cast<int>( i );
                }
            }
        }
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TableViewPushButton::TableViewPushButton( caf::FieldUiCapability* field, const QString& text, QWidget* parent /*= 0*/ )
    : QPushButton( text, parent )
    , m_fieldHandle( field )
{
    connect( this, SIGNAL( pressed() ), SLOT( slotPressed() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TableViewPushButton::slotPressed()
{
    if ( m_fieldHandle )
    {
        Variant val = m_fieldHandle->uiValue();
        if ( val.canConvert<bool>() )
        {
            bool currentValue = val.value<bool>();
            caf::UiCommandSystemProxy::instance()->setUiValueToField( m_fieldHandle, !currentValue );
        }
    }
}

} // end namespace caf
