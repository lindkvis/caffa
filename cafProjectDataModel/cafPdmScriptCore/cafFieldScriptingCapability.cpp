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
#include "cafFieldScriptingCapability.h"
#include "cafStringTools.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldScriptingCapabilityIOHandler<std::string>::writeToField( std::string&              fieldValue,
                                                                   std::istream&             inputStream,
                                                                   caf::PdmScriptIOMessages* errorMessageContainer,
                                                                   bool                      stringsAreQuoted )
{
    fieldValue = "";

    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
    std::string accumulatedFieldValue;

    char currentChar;
    bool validStringStart = !stringsAreQuoted;
    bool validStringEnd   = !stringsAreQuoted;
    if ( stringsAreQuoted )
    {
        currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
        if ( currentChar == char( '"' ) )
        {
            validStringStart = true;
        }
    }

    if ( validStringStart )
    {
        while ( true )
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
            if ( inputStream.eof() )
            {
                break;
            }
            else
            {
                if ( currentChar != char( '\\' ) )
                {
                    if ( currentChar == char( '"' ) ) // End Quote
                    {
                        // Reached end of string
                        validStringEnd = true;
                        break;
                    }
                    else
                    {
                        accumulatedFieldValue += currentChar;
                    }
                }
                else
                {
                    currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    accumulatedFieldValue += currentChar;
                }
            }
        }
    }
    if ( !validStringStart )
    {
        // Unexpected start of string, Missing '"'
        // Error message
        errorMessageContainer->addError(
            "String argument does not seem to be quoted. Missing the start '\"' in the \"" +
            errorMessageContainer->currentArgument + "\" argument of the command: \"" +
            errorMessageContainer->currentCommand + "\"" );
        // Could interpret as unquoted text
    }
    else if ( !validStringEnd )
    {
        // Unexpected end of string, Missing '"'
        // Error message
        errorMessageContainer->addError( "String argument does not seem to be quoted. Missing the end '\"' in the \"" +
                                         errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                         errorMessageContainer->currentCommand + "\"" );
        // Could interpret as unquoted text
    }

    if ( accumulatedFieldValue != "None" )
    {
        fieldValue = accumulatedFieldValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldScriptingCapabilityIOHandler<std::string>::readFromField( const std::string& fieldValue,
                                                                    std::ostream&      outputStream,
                                                                    bool               quoteStrings,
                                                                    bool               quoteNonBuiltin )
{
    if ( quoteStrings ) outputStream << "\"";
    outputStream << fieldValue;
    if ( quoteStrings ) outputStream << "\"";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldScriptingCapabilityIOHandler<bool>::writeToField( bool&                     fieldValue,
                                                            std::istream&             inputStream,
                                                            caf::PdmScriptIOMessages* errorMessageContainer,
                                                            bool                      stringsAreQuoted )
{
    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
    std::string accumulatedFieldValue;
    char        nextChar;
    char        currentChar;
    while ( !inputStream.eof() )
    {
        nextChar = errorMessageContainer->peekNextChar( inputStream );
        if ( std::isalpha( nextChar ) )
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
            accumulatedFieldValue += currentChar;
        }
        else
        {
            break;
        }
    }
    // Accept TRUE or False in any case combination.
    bool evaluatesToTrue  = caf::StringTools::tolower( accumulatedFieldValue ).compare( "true" ) == 0;
    bool evaluatesToFalse = caf::StringTools::tolower( accumulatedFieldValue ).compare( "false" ) == 0;
    if ( evaluatesToTrue == evaluatesToFalse )
    {
        std::string errorMessage = "Boolean argument '" + errorMessageContainer->currentArgument + "' for the command '" +
                                   errorMessageContainer->currentCommand + "' does not evaluate to either true or false";
        errorMessageContainer->addError( errorMessage );
    }
    fieldValue = evaluatesToTrue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldScriptingCapabilityIOHandler<bool>::readFromField( const bool&   fieldValue,
                                                             std::ostream& outputStream,
                                                             bool          quoteStrings,
                                                             bool          quoteNonBuiltin )
{
    // Lower-case true/false is used in the documentation.
    outputStream << ( fieldValue ? "true" : "false" );
}
