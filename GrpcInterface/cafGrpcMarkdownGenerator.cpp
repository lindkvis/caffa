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
#include "cafGrpcMarkdownGenerator.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafFieldDocumentationCapability.h"
#include "cafFieldHandle.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"
#include "cafObjectMethod.h"

#include <regex>

using namespace caffa;
using namespace caffa::rpc;

MarkdownGenerator::~MarkdownGenerator()
{
}

std::string MarkdownGenerator::name() const
{
    return "markdown";
}

std::string MarkdownGenerator::generate( std::list<std::unique_ptr<caffa::Document>>& documents )
{
    std::string code = "# ARU Server API\n\n";

    for ( auto& doc : documents )
    {
        code += generate( doc.get() );
    }
    return code;
}

std::string MarkdownGenerator::generate( ObjectHandle* object, bool objectMethodResultOrParameter )
{
    JsonSerializer serializer;
    CAFFA_DEBUG( "Generating code for class " << object->classKeyword() );
    std::vector<std::string> fieldDependencies;

    auto parentClassKeyword = findParentClass( object );
    fieldDependencies.push_back( parentClassKeyword );

    std::string code;
    code += "## " + std::string( object->classKeyword() ) + "\n";
    code += object->classDocumentation() + "\n";

    code += "### Fields\n";
    CAFFA_DEBUG( "Test" );
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            CAFFA_DEBUG( "field: " << field->keyword() );
            code += "- **" + field->keyword() + "** (*" + docDataType( field ) + "*)";
            CAFFA_DEBUG( "Found data type" );

            auto fieldDependency = dependency( field );
            if ( !fieldDependency.empty() )
            {
                fieldDependencies.push_back( fieldDependency );
            }
            else if ( objectMethodResultOrParameter )
            {
                CAFFA_DEBUG( "Creating json value" );
                auto jsonCap = field->capability<FieldJsonCapability>();
                if ( jsonCap )
                {
                    nlohmann::json json;
                    jsonCap->writeToJson( json, serializer );
                    CAFFA_DEBUG( "Got JSON: " << json.dump() );
                    if ( json.contains( "value" ) )
                    {
                        code += " = *" + json["value"].dump() + "*";
                    }
                }
            }
            auto doc = field->capability<FieldDocumentationCapability>();
            if ( doc )
            {
                code += ": " + doc->documentation();
            }

            CAFFA_DEBUG( "done with field" );
            code += "\n";
        }
    }
    code += "\n";

    CAFFA_DEBUG( "Test 2" );

    std::vector<std::string> methodDependencies;

    auto methodNames = ObjectMethodFactory::instance()->registeredMethodNames( object );

    if ( !methodNames.empty() )
    {
        code += "### Methods\n";
        for ( auto methodName : methodNames )
        {
            auto method = ObjectMethodFactory::instance()->createMethodInstance( object, methodName );
            code += generate( method.get(), methodDependencies );
        }
    }

    std::string dependencyCode;
    for ( auto className : methodDependencies )
    {
        if ( className != "Object" && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get(), true );
        }
    }

    for ( auto className : fieldDependencies )
    {
        if ( className != "Object" && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get() );
        }
    }
    return dependencyCode + code;
}

std::string MarkdownGenerator::findParentClass( ObjectHandle* object ) const
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

std::string MarkdownGenerator::dependency( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return std::string( childField->childClassKeyword() );
    }
    else if ( childArrayField )
    {
        return std::string( childArrayField->childClassKeyword() );
    }
    return "";
}

std::string MarkdownGenerator::docDataType( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return std::string( childField->childClassKeyword() );
    }
    else if ( childArrayField )
    {
        return std::string( childArrayField->childClassKeyword() ) + "[]";
    }

    return field->dataType();
}

std::string MarkdownGenerator::generate( FieldHandle* field, std::vector<std::string>& dependencies )
{
    std::string code;

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();

    if ( !( scriptability && ( scriptability->isReadable() || scriptability->isWritable() ) ) ) return code;

    code += "    def " + field->keyword() + "(self):\n";
    auto doc = field->capability<FieldDocumentationCapability>();
    if ( doc )
    {
        code += doc->documentation() + "\n\n";
    }

    auto fieldDependency = dependency( field );
    if ( !fieldDependency.empty() ) dependencies.push_back( fieldDependency );

    return code;
}

std::string MarkdownGenerator::generate( caffa::ObjectMethod* method, std::vector<std::string>& dependencies )
{
    std::string code;

    auto fields = method->fields();

    code += "- " + std::string( method->classKeyword() ) + ": " + method->classDocumentation() + "\n";

    std::string parametersCode;
    int         index = 1;
    for ( auto field : fields )
    {
        if ( field->keyword() == "uuid" ) continue;

        parametersCode += "        " + std::to_string( index ) + ". **" + field->keyword() + "**, type: *" +
                          docDataType( field ) + "*";
        auto fieldDependency = dependency( field );
        if ( !fieldDependency.empty() )
            dependencies.push_back( fieldDependency );
        else
        {
            auto jsonCap = field->capability<FieldJsonCapability>();
            if ( jsonCap )
            {
                nlohmann::json json;
                jsonCap->writeToJson( json, JsonSerializer() );
                parametersCode += ", default: " + json["value"].dump();
            }
        }
        index++;
        parametersCode += "\n";
    }
    if ( !parametersCode.empty() )
    {
        code += "    - Parameters:\n" + parametersCode;
    }

    auto resultObject = method->defaultResult();

    code += "    - Returns: *" + std::string( resultObject->classKeyword() ) + "*\n";
    code += "\n";
    return code;
}

bool markdownRegistered = caffa::rpc::CodeGeneratorFactory::instance()->registerCreator<caffa::rpc::MarkdownGenerator>(
    typeid( caffa::rpc::MarkdownGenerator ).hash_code() );
