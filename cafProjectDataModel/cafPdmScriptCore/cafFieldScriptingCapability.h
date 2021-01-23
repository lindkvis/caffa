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
#pragma once

#include "cafAbstractFieldScriptingCapability.h"
#include "cafAppEnum.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafPdmScriptIOMessages.h"
#include "cafPtrArrayField.h"
#include "cafPtrField.h"
#include "cafStringTools.h"

#include <iostream>
#include <string>
#include <type_traits>

#define CAF_InitScriptableField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitField( field,                                                                                \
                   keyword,                                                                              \
                   default,                                                                              \
                   uiName,                                                                               \
                   iconResourceName,                                                                     \
                   caf::AbstractFieldScriptingCapability::helpString( toolTip, keyword ),                \
                   whatsThis );                                                                          \
    caf::AddScriptingCapabilityToField( field, keyword )

#define CAF_InitScriptableFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitFieldNoDefault( field,                                                                       \
                            keyword,                                                                     \
                            uiName,                                                                      \
                            iconResourceName,                                                            \
                            caf::AbstractFieldScriptingCapability::helpString( toolTip, keyword ),       \
                            whatsThis );                                                                 \
    caf::AddScriptingCapabilityToField( field, keyword )

#define CAF_InitScriptableFieldWithScriptKeyword( field, keyword, scriptKeyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitField( field,                                                                                                                \
                   keyword,                                                                                                              \
                   default,                                                                                                              \
                   uiName,                                                                                                               \
                   iconResourceName,                                                                                                     \
                   caf::AbstractFieldScriptingCapability::helpString( toolTip, scriptKeyword ),                                          \
                   whatsThis );                                                                                                          \
    caf::AddScriptingCapabilityToField( field, scriptKeyword )

#define CAF_InitScriptableFieldWithScriptKeywordNoDefault( field, keyword, scriptKeyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_InitFieldNoDefault( field,                                                                                                       \
                            keyword,                                                                                                     \
                            uiName,                                                                                                      \
                            iconResourceName,                                                                                            \
                            caf::AbstractFieldScriptingCapability::helpString( toolTip, scriptKeyword ),                                 \
                            whatsThis );                                                                                                 \
    caf::AddScriptingCapabilityToField( field, scriptKeyword )

namespace caf
{
template <typename DataType>
struct FieldScriptingCapabilityIOHandler
{
    static void writeToField( DataType&            fieldValue,
                              std::istream&        inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
    {
        inputStream >> fieldValue;
        if ( !inputStream.good() )
        {
            errorMessageContainer->addError( "Argument value is unreadable in the argument: \"" +
                                             errorMessageContainer->currentArgument + "\" in the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );

            inputStream.setstate( std::ios_base::goodbit );
        }
    }

    static void readFromField( const DataType& fieldValue,
                               std::ostream&   outputStream,
                               bool            quoteStrings     = true,
                               bool            quoteNonBuiltins = false )
    {
        outputStream << fieldValue;
    }
};

template <>
struct FieldScriptingCapabilityIOHandler<std::string>
{
    static void writeToField( std::string&         fieldValue,
                              std::istream&        inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true );
    static void readFromField( const std::string& fieldValue,
                               std::ostream&      outputStream,
                               bool               quoteStrings     = true,
                               bool               quoteNonBuiltins = false );
};

template <>
struct FieldScriptingCapabilityIOHandler<bool>
{
    static void writeToField( bool&                fieldValue,
                              std::istream&        inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true );
    static void readFromField( const bool&   fieldValue,
                               std::ostream& outputStream,
                               bool          quoteStrings     = true,
                               bool          quoteNonBuiltins = false );
};

template <typename T>
struct FieldScriptingCapabilityIOHandler<AppEnum<T>>
{
    static void writeToField( AppEnum<T>&          fieldValue,
                              std::istream&        inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
        std::string accumulatedFieldValue;
        char        nextChar;
        char        currentChar;
        while ( !inputStream.eof() )
        {
            nextChar = errorMessageContainer->peekNextChar( inputStream );
            if ( std::isalpha(nextChar) || nextChar == char( '_' ) )
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                accumulatedFieldValue += currentChar;
            }
            else
            {
                break;
            }
        }
        if ( !fieldValue.setFromText( accumulatedFieldValue ) )
        {
            // Unexpected enum value
            // Error message
            errorMessageContainer->addError( "Argument must be valid enum value. " +
                                             errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );
        }
    }

    static void readFromField( const AppEnum<T>& fieldValue,
                               std::ostream&     outputStream,
                               bool              quoteStrings     = true,
                               bool              quoteNonBuiltins = false )
    {
        if ( quoteNonBuiltins )
        {
            outputStream << "\"" << fieldValue << "\"";
        }
        else
        {
            outputStream << fieldValue;
        }
    }
};

template <typename T>
struct FieldScriptingCapabilityIOHandler<std::vector<T>>
{
    static void writeToField( std::vector<T>&      fieldValue,
                              std::istream&        inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
        char chr = errorMessageContainer->readCharWithLineNumberCount( inputStream );
        if ( chr == char( '[' ) )
        {
            while ( !inputStream.eof() )
            {
                errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                char nextChar = errorMessageContainer->peekNextChar( inputStream );
                if ( nextChar == char( ']' ) )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    break;
                }
                else if ( nextChar == char( ',' ) )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                }

                T value;
                FieldScriptingCapabilityIOHandler<T>::writeToField( value, inputStream, errorMessageContainer, true );
                fieldValue.push_back( value );
            }
        }
        else
        {
            errorMessageContainer->addError( "Array argument is missing start '['. " +
                                             errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );
        }
    }

    static void readFromField( const std::vector<T>& fieldValue,
                               std::ostream&         outputStream,
                               bool                  quoteStrings     = true,
                               bool                  quoteNonBuiltins = false )
    {
        outputStream << "[";
        for ( size_t i = 0; i < fieldValue.size(); ++i )
        {
            FieldScriptingCapabilityIOHandler<T>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

template <typename T>
struct FieldScriptingCapabilityIOHandler<std::vector<T*>>
{
    static void writeToField( std::vector<T*>&       fieldValue,
                              const std::vector<T*>& allObjectsOfType,
                              std::istream&          inputStream,
                              PdmScriptIOMessages*   errorMessageContainer )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
        char chr = errorMessageContainer->readCharWithLineNumberCount( inputStream );
        if ( chr == '[' )
        {
            std::vector<std::string> allValues;
            std::string              currentValue;
            while ( !inputStream.eof() )
            {
                errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                char nextChar = errorMessageContainer->peekNextChar( inputStream );
                if ( nextChar == ']' )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    break;
                }
                else if ( nextChar == ',' )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                    if ( !currentValue.empty() ) allValues.push_back( currentValue );
                    currentValue = "";
                }
                else
                {
                    currentValue += errorMessageContainer->readCharWithLineNumberCount( inputStream );
                }
                for ( std::string textValue : allValues )
                {
                    std::istringstream sstream( textValue );
                    T*                 singleValue;
                    FieldScriptingCapabilityIOHandler<T*>::writeToField( singleValue,
                                                                         allObjectsOfType,
                                                                         sstream,
                                                                         errorMessageContainer );
                    fieldValue.push_back( singleValue );
                }
            }
        }
        else
        {
            errorMessageContainer->addError( "Array argument is missing start '['. " +
                                             errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );
        }
    }

    static void readFromField( const std::vector<T*>& fieldValue,
                               std::ostream&          outputStream,
                               bool                   quoteStrings     = true,
                               bool                   quoteNonBuiltins = false )
    {
        outputStream << "[";
        for ( size_t i = 0; i < fieldValue.size(); ++i )
        {
            FieldScriptingCapabilityIOHandler<T*>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

template <typename DataType>
struct FieldScriptingCapabilityIOHandler<DataType*>
{
    static void writeToField( DataType*&                    fieldValue,
                              const std::vector<DataType*>& allObjectsOfType,
                              std::istream&                 inputStream,
                              PdmScriptIOMessages*          errorMessageContainer )
    {
        fieldValue = nullptr; // Default initialized to nullptr

        std::string fieldString;
        bool        stringsAreQuoted = false;
        FieldScriptingCapabilityIOHandler<std::string>::writeToField( fieldString,
                                                                      inputStream,
                                                                      errorMessageContainer,
                                                                      stringsAreQuoted );

        if ( !inputStream.good() )
        {
            errorMessageContainer->addError( "Argument value is unreadable in the argument: \"" +
                                             errorMessageContainer->currentArgument + "\" in the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );

            inputStream.setstate( std::ios_base::goodbit );
            return;
        }

        if ( fieldString.empty() ) return;

        std::list<std::string> classAndAddress = caf::StringTools::split(fieldString, ":" );
        CAF_ASSERT( classAndAddress.size() == 2 );

        unsigned long long address = std::stoull( classAndAddress.back() );
        for ( DataType* object : allObjectsOfType )
        {
            if ( reinterpret_cast<unsigned long long>( object ) == address )
            {
                fieldValue = object;
                break;
            }
        }
    }

    static void readFromField( const DataType* fieldValue,
                               std::ostream&   outputStream,
                               bool            quoteStrings     = true,
                               bool            quoteNonBuiltins = false )
    {
        if ( fieldValue )
        {
            std::string textOutput = DataType::classKeywordStatic() + ":" +
                                     std::to_string( reinterpret_cast<uint64_t>( fieldValue ) );
            if ( quoteNonBuiltins )
            {
                outputStream << "\"" + textOutput << "\"";
            }
            else
            {
                outputStream << textOutput;
            }
        }
    }
};

//==================================================================================================
//
//
//
//==================================================================================================
template <typename FieldType>
class FieldScriptingCapability : public AbstractFieldScriptingCapability
{
public:
    FieldScriptingCapability( FieldType* field, const std::string& fieldName, bool giveOwnership )
        : AbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( std::istream&        inputStream,
                       ObjectFactory*       objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted    = true,
                       caf::ObjectHandle*   existingObjectsRoot = nullptr ) override
    {
        typename FieldType::FieldDataType value;
        FieldScriptingCapabilityIOHandler<typename FieldType::FieldDataType>::writeToField( value,
                                                                                            inputStream,
                                                                                            errorMessageContainer,
                                                                                            stringsAreQuoted );

        if ( this->isIOWriteable() )
        {
            m_field->setValue( value );
        }
    }

    void readFromField( std::ostream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        FieldScriptingCapabilityIOHandler<typename FieldType::FieldDataType>::readFromField( m_field->value(),
                                                                                             outputStream,
                                                                                             quoteStrings,
                                                                                             quoteNonBuiltins );
    }

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldScriptingCapability<PtrField<DataType*>> : public AbstractFieldScriptingCapability
{
public:
    FieldScriptingCapability( PtrField<DataType*>* field, const std::string& fieldName, bool giveOwnership )
        : AbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( std::istream&        inputStream,
                       ObjectFactory*       objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted    = true,
                       caf::ObjectHandle*   existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        DataType* object = nullptr;
        FieldScriptingCapabilityIOHandler<DataType*>::writeToField( object,
                                                                    allObjectsOfType,
                                                                    inputStream,
                                                                    errorMessageContainer );

        if ( object && this->isIOWriteable() )
        {
            m_field->setValue( object );
        }
    }

    void readFromField( std::ostream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        FieldScriptingCapabilityIOHandler<DataType*>::readFromField( m_field->value(),
                                                                     outputStream,
                                                                     quoteStrings,
                                                                     quoteNonBuiltins );
    }

private:
    PtrField<DataType*>* m_field;
};

template <typename DataType>
class FieldScriptingCapability<ChildField<DataType*>> : public AbstractFieldScriptingCapability
{
public:
    FieldScriptingCapability( ChildField<DataType*>* field, const std::string& fieldName, bool giveOwnership )
        : AbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( std::istream&        inputStream,
                       ObjectFactory*       objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted    = true,
                       caf::ObjectHandle*   existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        DataType* object = nullptr;
        FieldScriptingCapabilityIOHandler<DataType*>::writeToField( object,
                                                                    allObjectsOfType,
                                                                    inputStream,
                                                                    errorMessageContainer );

        if ( object && this->isIOWriteable() )
        {
            m_field->setValue( object );
        }
    }

    void readFromField( std::ostream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        FieldScriptingCapabilityIOHandler<DataType*>::readFromField( m_field->value(),
                                                                     outputStream,
                                                                     quoteStrings,
                                                                     quoteNonBuiltins );
    }

private:
    ChildField<DataType*>* m_field;
};

template <typename DataType>
class FieldScriptingCapability<PtrArrayField<DataType*>> : public AbstractFieldScriptingCapability
{
public:
    FieldScriptingCapability( PtrArrayField<DataType*>* field, const std::string& fieldName, bool giveOwnership )
        : AbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( std::istream&        inputStream,
                       ObjectFactory*       objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted    = true,
                       caf::ObjectHandle*   existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        std::vector<DataType*> objects;
        FieldScriptingCapabilityIOHandler<std::vector<DataType*>>::writeToField( objects,
                                                                                 allObjectsOfType,
                                                                                 inputStream,
                                                                                 errorMessageContainer );

        if ( this->isIOWriteable() )
        {
            m_field->setValue( objects );
        }
    }

    void readFromField( std::ostream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        FieldScriptingCapabilityIOHandler<std::vector<DataType*>>::readFromField( m_field->ptrReferencedObjects(),
                                                                                  outputStream,
                                                                                  quoteStrings,
                                                                                  quoteNonBuiltins );
    }

private:
    PtrArrayField<DataType*>* m_field;
};

template <typename DataType>
class FieldScriptingCapability<ChildArrayField<DataType*>> : public AbstractFieldScriptingCapability
{
public:
    FieldScriptingCapability( ChildArrayField<DataType*>* field, const std::string& fieldName, bool giveOwnership )
        : AbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( std::istream&        inputStream,
                       ObjectFactory*       objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted    = true,
                       caf::ObjectHandle*   existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        std::vector<DataType*> objects;
        FieldScriptingCapabilityIOHandler<std::vector<DataType*>>::writeToField( objects,
                                                                                 allObjectsOfType,
                                                                                 inputStream,
                                                                                 errorMessageContainer );

        if ( this->isIOWriteable() )
        {
            m_field->setValue( objects );
        }
    }

    void readFromField( std::ostream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        FieldScriptingCapabilityIOHandler<std::vector<DataType*>>::readFromField( m_field->childObjects(),
                                                                                  outputStream,
                                                                                  quoteStrings,
                                                                                  quoteNonBuiltins );
    }

private:
    ChildArrayField<DataType*>* m_field;
};

template <typename FieldType>
void AddScriptingCapabilityToField( FieldType* field, const std::string& fieldName )
{
    if ( field->template capability<FieldScriptingCapability<FieldType>>() == nullptr )
    {
        new FieldScriptingCapability<FieldType>( field, fieldName, true );
    }
}

} // namespace caf
