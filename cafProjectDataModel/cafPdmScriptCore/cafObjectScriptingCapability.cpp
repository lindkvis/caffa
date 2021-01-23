//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafObjectScriptingCapability.h"

#include "cafAbstractFieldScriptingCapability.h"
#include "cafFieldIoCapability.h"
#include "cafObject.h"
#include "cafObjectHandle.h"
#include "cafPdmScriptIOMessages.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectScriptingCapability::ObjectScriptingCapability( ObjectHandle* owner, bool giveOwnership )
    : m_owner( owner )
{
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectScriptingCapability::~ObjectScriptingCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectScriptingCapability::readFields( std::istream&        inputStream,
                                            ObjectFactory*       objectFactory,
                                            PdmScriptIOMessages* errorMessageContainer )
{
    std::set<std::string> readFields;
    bool                  isLastArgumentRead = false;
    while ( !inputStream.eof() && !isLastArgumentRead )
    {
        // Read field keyword
        bool        fieldDataFound       = false;
        bool        isEndOfArgumentFound = false;
        std::string keyword;
        {
            errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
            {
                char currentChar;
                while ( !inputStream.eof() )
                {
                    currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );

                    if ( currentChar == '=' || currentChar == ')' || currentChar == ',' || currentChar == ' ' )
                    {
                        break;
                    }
                    else
                    {
                        keyword += currentChar;
                    }
                }

                if ( currentChar == ' ' )
                {
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                    currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                }

                if ( currentChar == char( '=' ) )
                {
                    fieldDataFound = true;
                }
                else if ( currentChar == char( ')' ) )
                {
                    if ( !keyword.empty() )
                    {
                        errorMessageContainer->addError(
                            std::string( "Can't find the '=' after the argument named '" ) + keyword +
                            "' in the command '" + errorMessageContainer->currentCommand + "'" );
                    }
                    isLastArgumentRead = true;
                }
                else if ( currentChar == char( ',' ) )
                {
                    errorMessageContainer->addError( std::string( "Can't find the '=' after the argument named '" ) +
                                                     keyword + "' in the command '" +
                                                     errorMessageContainer->currentCommand + "'" );
                    isEndOfArgumentFound = true;
                }
                else
                {
                    errorMessageContainer->addError( std::string( "Can't find the '=' after the argument named '" ) +
                                                     keyword + "' in the command '" +
                                                     errorMessageContainer->currentCommand + "'" );
                }
            }

            if ( readFields.count( keyword ) )
            {
                // Warning message: Referenced the same argument several times
                errorMessageContainer->addWarning( "The argument: \"" + keyword +
                                                   "\" is referenced several times in the command: \"" +
                                                   errorMessageContainer->currentCommand + "\"" );
            }
        }

        if ( fieldDataFound )
        {
            // Make field read its data

            FieldHandle* fieldHandle = m_owner->findField( keyword );
            if ( fieldHandle && fieldHandle->capability<FieldIoCapability>() &&
                 fieldHandle->capability<AbstractFieldScriptingCapability>() )
            {
                auto                              ioFieldHandle = fieldHandle->capability<FieldIoCapability>();
                AbstractFieldScriptingCapability* scriptability =
                    fieldHandle->capability<AbstractFieldScriptingCapability>();

                if ( ioFieldHandle->isIOReadable() )
                {
                    errorMessageContainer->currentArgument = keyword;
                    scriptability->writeToField( inputStream, objectFactory, errorMessageContainer );
                    errorMessageContainer->currentArgument = keyword;
                }
            }
            else
            {
                // Error message: Unknown argument name
                errorMessageContainer->addWarning( "The argument: \"" + keyword + "\" does not exist in the command: \"" +
                                                   errorMessageContainer->currentCommand + "\"" );
            }
        }

        // Skip to end of argument ',' or end of call ')'
        if ( !( isLastArgumentRead || isEndOfArgumentFound ) )
        {
            char currentChar;
            bool isOutsideQuotes = true;
            while ( !inputStream.eof() )
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                if ( isOutsideQuotes )
                {
                    if ( currentChar == char( ',' ) )
                    {
                        break;
                    }

                    if ( currentChar == char( ')' ) )
                    {
                        isLastArgumentRead = true;
                        break;
                    }
                    if ( currentChar == char( '\"' ) )
                    {
                        isOutsideQuotes = false;
                    }
                }
                else
                {
                    if ( currentChar == char( '\"' ) )
                    {
                        isOutsideQuotes = true;
                    }

                    if ( currentChar == char( '\\' ) )
                    {
                        currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectScriptingCapability::writeFields( std::ostream& outputStream ) const
{
    std::vector<FieldHandle*> fields;
    m_owner->fields( fields );
    int writtenFieldCount = 0;
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const auto                              ioField       = fields[it]->capability<FieldIoCapability>();
        const AbstractFieldScriptingCapability* scriptability = fields[it]->capability<AbstractFieldScriptingCapability>();
        if ( scriptability && ioField && ioField->isIOWritable() )
        {
            std::string keyword = ioField->fieldHandle()->keyword();
            CAF_ASSERT( ObjectIoCapability::isValidElementName( keyword ) );

            if ( writtenFieldCount >= 1 )
            {
                outputStream << ", ";
            }

            outputStream << keyword << " = ";
            scriptability->readFromField( outputStream );

            writtenFieldCount++;
        }
    }
}
