//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015 Ceetron Solutions AS
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

#include "cafPdmSettings.h"

#include "cafField.h"
#include "cafObjectHandle.h"
#include "cafObjectXmlCapability.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmSettings::readFieldsFromApplicationStore( caf::ObjectHandle* object, const QString context )
{
    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(),
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings                         settings;
    std::vector<caf::FieldHandle*> fields;

    object->fields( fields );
    size_t i;
    for ( i = 0; i < fields.size(); i++ )
    {
        caf::FieldHandle* fieldHandle = fields[i];

        std::vector<caf::ObjectHandle*> children;
        fieldHandle->childObjects( &children );
        for ( size_t childIdx = 0; childIdx < children.size(); childIdx++ )
        {
            caf::ObjectHandle* child        = children[childIdx];
            auto                  ioCapability = child->capability<ObjectIoCapability>();

            QString subContext = context + ioCapability->classKeyword() + "/";
            readFieldsFromApplicationStore( child, subContext );
        }

        if ( children.size() == 0 )
        {
            QString key = context + fieldHandle->keyword();
            if ( settings.contains( key ) )
            {
                QVariant val = settings.value( key );

                caf::ValueField* valueField = dynamic_cast<caf::ValueField*>( fieldHandle );
                CAF_ASSERT( valueField );
                valueField->setFromQVariant( val );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmSettings::writeFieldsToApplicationStore( const caf::ObjectHandle* object, const QString context )
{
    CAF_ASSERT( object );

    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(),
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    std::vector<caf::FieldHandle*> fields;
    object->fields( fields );

    size_t i;
    for ( i = 0; i < fields.size(); i++ )
    {
        caf::FieldHandle* fieldHandle = fields[i];

        std::vector<caf::ObjectHandle*> children;
        fieldHandle->childObjects( &children );
        for ( size_t childIdx = 0; childIdx < children.size(); childIdx++ )
        {
            caf::ObjectHandle* child = children[childIdx];
            QString               subContext;
            if ( context.isEmpty() )
            {
                auto objHandle = child->capability<ObjectIoCapability>();
                subContext     = objHandle->classKeyword() + "/";
            }

            writeFieldsToApplicationStore( child, subContext );
        }

        if ( children.size() == 0 )
        {
            caf::ValueField* valueField = dynamic_cast<caf::ValueField*>( fieldHandle );
            CAF_ASSERT( valueField );
            settings.setValue( context + fieldHandle->keyword(), valueField->toQVariant() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmSettings::readValueFieldsFromApplicationStore( caf::ObjectHandle* object, const QString folderName /*= ""*/ )
{
    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(),
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    if ( folderName != "" )
    {
        settings.beginGroup( folderName );
    }

    std::vector<caf::FieldHandle*> fields;

    object->fields( fields );
    size_t i;
    for ( i = 0; i < fields.size(); i++ )
    {
        caf::FieldHandle* fieldHandle = fields[i];
        caf::ValueField*  valueField  = dynamic_cast<caf::ValueField*>( fieldHandle );

        if ( valueField )
        {
            QString key = fieldHandle->keyword();
            if ( settings.contains( key ) )
            {
                QVariant val = settings.value( key );

                QString          fieldText = "<Element>" + val.toString() + "</Element>";
                QXmlStreamReader reader( fieldText );

                // Make stream point to the text data for the field
                reader.readNext(); // StartDocument
                reader.readNext(); // StartElement
                reader.readNext(); // Characters
                fieldHandle->capability<FieldIoCapability>()->readFieldData( reader, nullptr );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmSettings::writeValueFieldsToApplicationStore( const caf::ObjectHandle* object,
                                                      const QString               folderName /*= ""*/ )
{
    CAF_ASSERT( object );

    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(),
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    if ( folderName != "" )
    {
        settings.beginGroup( folderName );
    }

    std::vector<caf::FieldHandle*> fields;
    object->fields( fields );

    size_t i;
    for ( i = 0; i < fields.size(); i++ )
    {
        caf::FieldHandle* fieldHandle = fields[i];
        caf::ValueField*  valueField  = dynamic_cast<caf::ValueField*>( fieldHandle );
        if ( valueField )
        {
            QString          fieldText;
            QXmlStreamWriter writer( &fieldText );

            fieldHandle->capability<FieldIoCapability>()->writeFieldData( writer );
            settings.setValue( fieldHandle->keyword(), fieldText );
        }
    }
}

} // namespace caf
