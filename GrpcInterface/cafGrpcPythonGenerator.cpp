// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
#include "cafGrpcPythonGenerator.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafFieldDocumentationCapability.h"
#include "cafFieldHandle.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafMethodHandle.h"
#include "cafObjectHandle.h"
#include "cafStringTools.h"

#include <regex>

using namespace caffa;
using namespace caffa::rpc;

PythonGenerator::~PythonGenerator()
{
}

std::string PythonGenerator::name() const
{
    return "python";
}

std::string PythonGenerator::generate( std::list<std::shared_ptr<caffa::Document>>& documents )
{
    std::string code = "from caffa import Object, Document\n\n\n";

    for ( auto& doc : documents )
    {
        code += generate( doc );
    }
    return code;
}

std::string PythonGenerator::generate( std::shared_ptr<ObjectHandle> object )
{
    CAFFA_DEBUG( "Generating code for class " << object->classKeyword() );

    std::vector<std::string> dependencies;

    auto parentClassKeyword = findParentClass( object );
    dependencies.push_back( parentClassKeyword );

    std::string objectCode;
    objectCode += "class " + std::string( object->classKeyword() ) + "(" + parentClassKeyword + "):\n";
    if ( !object->classDocumentation().empty() )
    {
        objectCode += "    \"\"\"" + object->classDocumentation() + "\"\"\"\n\n";
    }
    objectCode += "    def __init__(self, object):\n";
    objectCode += "        self._json_object = object._json_object\n";
    objectCode += "        self._session_uuid = object._session_uuid\n";
    objectCode += "        self._object_cache = object._object_cache\n";
    objectCode += "        self._channel = object._channel\n";
    objectCode += "        self._field_stub = object._field_stub\n";
    objectCode += "        self._object_stub = object._object_stub\n\n";

    for ( auto field : object->fields() )
    {
        if ( field->capability<caffa::FieldScriptingCapability>() != nullptr )
        {
            objectCode += generate( field, dependencies );
        }
    }

    auto methods = object->methods();

    for ( auto method : methods )
    {
        objectCode += generate( method );
    }
    std::string dependencyCode;

    for ( auto className : dependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject );
        }
    }

    return dependencyCode + objectCode;
}

bool PythonGenerator::isBuiltInClass( const std::string& classKeyword ) const
{
    return classKeyword == "Object" || classKeyword == "Document";
}

std::string PythonGenerator::findParentClass( std::shared_ptr<ObjectHandle> object ) const
{
    auto inheritanceStack = object->classInheritanceStack();
    for ( size_t i = 1; i < inheritanceStack.size(); ++i )
    {
        auto object = DefaultObjectFactory::instance()->create( inheritanceStack[i] );
        if ( object )
        {
            return std::string( inheritanceStack[i] );
        }
    }
    return "Object";
}

std::string PythonGenerator::pythonValue( const std::string& cppValue ) const
{
    if ( cppValue == "true" )
        return "True";
    else if ( cppValue == "false" )
        return "False";
    else if ( cppValue == "null" )
        return "None";

    return cppValue;
}

std::string PythonGenerator::castFieldValue( const caffa::FieldHandle* field, const std::string& value ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return childArrayField->childClassKeyword() + "(" + value + ")";
    }
    else if ( childArrayField )
    {
        return childArrayField->childClassKeyword() + "(" + value + ")";
    }

    return value;
}

std::string PythonGenerator::dependency( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return childField->childClassKeyword();
    }
    else if ( childArrayField )
    {
        return childArrayField->childClassKeyword();
    }
    return "";
}

std::string PythonGenerator::pythonDataType( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return childField->childClassKeyword();
    }
    else if ( childArrayField )
    {
        return childArrayField->childClassKeyword();
    }

    auto dataType = field->dataType();
    dataType      = StringTools::replace( dataType, "string", "str" );
    dataType      = StringTools::replace( dataType, "uint", "int" );
    dataType      = StringTools::replace( dataType, "64", "" );
    dataType      = StringTools::replace( dataType, "32", "" );
    dataType      = StringTools::replace( dataType, "bool", "boolean" );
    dataType      = StringTools::replace( dataType, "double", "float" );
    return dataType;
}

std::string PythonGenerator::generate( FieldHandle* field, std::vector<std::string>& dependencies )
{
    std::string code;

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();

    if ( !( scriptability && ( scriptability->isReadable() || scriptability->isWritable() ) ) ) return code;

    code += "    @property\n";
    code += "    def " + field->keyword() + "(self):\n";
    auto doc = field->capability<FieldDocumentationCapability>();
    if ( doc )
    {
        code += "        \"\"\"" + doc->documentation() + "\"\"\"\n\n";
    }
    if ( scriptability->isReadable() )
    {
        code += "        return " + castFieldValue( field, "self.get(\"" + field->keyword() + "\")" ) + "\n\n";
    }
    else
    {
        code += "        raise AttributeError(\"" + field->keyword() + " is write-only\")\n\n";
    }
    if ( scriptability->isWritable() )
    {
        code += "    @" + field->keyword() + ".setter\n";
        code += "    def " + field->keyword() + "(self, value):\n";
        code += "        return self.set(\"" + field->keyword() + "\", value)\n\n";
    }

    auto fieldDependency = dependency( field );
    if ( !fieldDependency.empty() ) dependencies.push_back( fieldDependency );

    return code;
}

std::string PythonGenerator::generate( caffa::MethodHandle* method )
{
    return "";
}

bool pythonRegistered = caffa::rpc::CodeGeneratorFactory::instance()->registerCreator<caffa::rpc::PythonGenerator>(
    typeid( caffa::rpc::PythonGenerator ).hash_code() );
