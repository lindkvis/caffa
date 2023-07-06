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
#include "cafPythonGenerator.h"

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

    for ( auto doc : documents )
    {
        code += generate( doc.get() );
    }
    return code;
}

std::string PythonGenerator::generate( const ObjectHandle* object, bool passByValue )
{
    CAFFA_DEBUG( "Generating code for class " << object->classKeyword() );

    std::string localTag = passByValue ? "True" : "False";

    std::vector<std::string> fieldDependencies;

    auto parentClassKeyword = findParentClass( object );
    fieldDependencies.push_back( parentClassKeyword );

    std::string objectCode;
    objectCode += "class " + object->classKeyword() + "(" + parentClassKeyword + "):\n";
    if ( !object->classDocumentation().empty() )
    {
        objectCode += "    \"\"\"" + object->classDocumentation() + "\"\"\"\n\n";
    }
    objectCode += "    def __init__(self, json_object=\"\", session_uuid=\"\", grpc_channel=None, local=" + localTag +
                  "):\n";
    objectCode += "        super().__init__(json_object, session_uuid, grpc_channel, local)\n";
    objectCode += "\n";

    if ( passByValue )
    {
        objectCode += generateCreateMethod( object );
    }

    for ( auto field : object->fields() )
    {
        if ( passByValue || field->capability<caffa::FieldScriptingCapability>() != nullptr )
        {
            objectCode += generate( field, passByValue, fieldDependencies );
        }
    }

    std::vector<std::string> methodDependencies;
    auto                     methods = object->methods();

    for ( auto method : methods )
    {
        CAFFA_INFO( "Generating method: " << method->name() );
        objectCode += generate( method, methodDependencies );
    }
    std::string dependencyCode;

    for ( auto className : methodDependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get(), true );
        }
    }
    for ( auto className : fieldDependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get() );
        }
    }

    return dependencyCode + objectCode;
}

std::string PythonGenerator::generateCreateMethod( const ObjectHandle* object )
{
    std::vector<std::string> dependencies;

    std::string code;

    code += "    @classmethod\n";
    code += "    def create(cls, session_uuid = \"\", grpc_channel = None";

    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += ", " + field->keyword();
            auto childField      = dynamic_cast<ChildFieldHandle*>( field );
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );

            if ( childField )
            {
                dependencies.push_back( childField->childClassKeyword() );
                code += ": " + childField->childClassKeyword() + " = None";
            }
            else if ( childArrayField )
            {
                dependencies.push_back( childArrayField->childClassKeyword() );
                code += ": " + childArrayField->childClassKeyword() + " = []";
            }
            else
            {
                auto jsonCap = field->capability<FieldJsonCapability>();
                if ( jsonCap )
                {
                    nlohmann::json json;
                    jsonCap->writeToJson( json, JsonSerializer() );
                    code += " = " + pythonValue( json["value"].dump() );
                }
            }
        }
    }
    code += "):\n";
    code += "        self = cls(json_object=\"\", session_uuid=session_uuid, grpc_channel=grpc_channel, local=True)\n";
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "        self.create_field(keyword=\"" + field->keyword() + "\",type=\"" + field->dataType() +
                    "\", value=" + field->keyword() + ")\n";
        }
    }
    code += "        return self\n";
    code += "\n";

    return code;
}

bool PythonGenerator::isBuiltInClass( const std::string& classKeyword ) const
{
    return classKeyword == "Object" || classKeyword == "Document";
}

std::string PythonGenerator::findParentClass( const ObjectHandle* object ) const
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
        return childField->childClassKeyword() + "(" + value + ")";
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

std::string PythonGenerator::pythonDataType( const std::string& caffaDataType ) const
{
    auto dataType = caffaDataType;
    dataType      = StringTools::replace( dataType, "string", "str" );
    dataType      = StringTools::replace( dataType, "uint", "int" );
    dataType      = StringTools::replace( dataType, "64", "" );
    dataType      = StringTools::replace( dataType, "32", "" );
    dataType      = StringTools::replace( dataType, "bool", "boolean" );
    dataType      = StringTools::replace( dataType, "double", "float" );
    return dataType;
}

std::string
    PythonGenerator::generate( const caffa::FieldHandle* field, bool passByValue, std::vector<std::string>& dependencies )
{
    std::string code;

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();

    bool writable = passByValue || ( scriptability && scriptability->isWritable() );
    bool readable = passByValue || ( scriptability && scriptability->isReadable() );

    code += "    @property\n";
    code += "    def " + field->keyword() + "(self):\n";
    auto doc = field->capability<FieldDocumentationCapability>();
    if ( doc )
    {
        code += "        \"\"\"" + doc->documentation() + "\"\"\"\n\n";
    }
    if ( readable )
    {
        code += "        return " + castFieldValue( field, "self.get(\"" + field->keyword() + "\")" ) + "\n\n";
    }
    else
    {
        code += "        raise AttributeError(\"" + field->keyword() + " is write-only\")\n\n";
    }
    if ( writable )
    {
        code += "    @" + field->keyword() + ".setter\n";
        code += "    def " + field->keyword() + "(self, value):\n";
        code += "        return self.set(\"" + field->keyword() + "\", value)\n\n";
    }

    auto fieldDependency = dependency( field );
    if ( !fieldDependency.empty() ) dependencies.push_back( fieldDependency );

    return code;
}

std::string PythonGenerator::generate( const caffa::MethodHandle* method, std::vector<std::string>& dependencies )
{
    auto jsonMethod = nlohmann::json::parse( method->schema() );

    CAFFA_INFO( jsonMethod );

    std::string code = "    def " + jsonMethod["keyword"].get<std::string>() + "(self";
    if ( jsonMethod.contains( "arguments" ) )
    {
        for ( auto argument : jsonMethod["arguments"] )
        {
            code += ", " + argument["keyword"].get<std::string>();
            auto dependency = argument["type"].get<std::string>();
            auto split      = caffa::StringTools::split( dependency, "::" );
            if ( split.size() == 2 )
            {
                dependencies.push_back( split.back() );
            }
        }
    }

    code += "):\n";

    if ( !method->documentation().empty() )
    {
        code += "        \"\"\"" + method->documentation() + "\n";
    }

    if ( jsonMethod.contains( "arguments" ) )
    {
        std::string parametersCode;
        parametersCode += "        Parameters\n";
        size_t argCount = 0;

        for ( auto argument : jsonMethod["arguments"] )
        {
            parametersCode += "        " + argument["keyword"].get<std::string>() + " : " +
                              pythonDataType( argument["type"].get<std::string>() ) + "\n";
            argCount++;
        }
        code += +"\n" + parametersCode;
    }

    bool        returnTypeIsObject = false;
    std::string returnType         = "";

    if ( jsonMethod.contains( "returns" ) )
    {
        auto returnValue = jsonMethod["returns"].get<std::string>();
        if ( returnValue != "void" )
        {
            returnType = jsonMethod["returns"].get<std::string>();

            std::string returnsCode;
            returnsCode += "        Returns: " + returnType + "\n";
            code += +"\n" + returnsCode;

            auto dependency = jsonMethod["returns"].get<std::string>();
            auto split      = caffa::StringTools::split( dependency, "::" );
            if ( split.size() == 2 )
            {
                returnTypeIsObject = true;
                returnType         = split.back();
                dependencies.push_back( returnType );
            }
        }

        code += "\n        \"\"\"\n\n";
    }

    code += "        method = self.method(\"" + method->name() + "\")\n";
    for ( auto argument : method->argumentNames() )
    {
        code += "        method.set_argument(\"" + argument + "\", " + argument + ")\n";
    }

    if ( returnTypeIsObject )
    {
        code +=
            "        return " + returnType +
            "(json_object=self.execute(method), session_uuid=self.session_uuid(), grpc_channel=self.grpc_channel(), "
            "local=True)\n\n";
    }
    else
    {
        code += "        return " + pythonDataType( returnType ) + "(self.execute(method))\n\n";
    }
    return code;
}

bool pythonRegistered = caffa::CodeGeneratorFactory::instance()->registerCreator<caffa::PythonGenerator>(
    typeid( caffa::PythonGenerator ).hash_code() );
