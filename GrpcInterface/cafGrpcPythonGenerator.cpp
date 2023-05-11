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
#include "cafObjectHandle.h"
#include "cafObjectMethod.h"
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

template <typename ChildFieldType>
std::string childClassKeyword( ChildFieldType& field )
{
    return std::string( field->childClassKeyword() );
}

std::string PythonGenerator::generate( std::list<std::shared_ptr<caffa::Document>>& documents )
{
    std::string code = "from caffa import Object, Document\n\n\n";

    for ( auto& doc : documents )
    {
        code += generate( doc.get() );
    }
    return code;
}

std::string PythonGenerator::generate( ObjectHandle* object, bool objectMethodResultOrParameter )
{
    CAFFA_DEBUG( "Generating code for class " << object->classKeyword() );
    if ( objectMethodResultOrParameter )
    {
        return generateObjectMethodField( object );
    }

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

    std::vector<std::string> methodDependencies;

    auto methodNames = ObjectMethodFactory::instance()->registeredMethodNames( object );

    for ( auto methodName : methodNames )
    {
        auto method = ObjectMethodFactory::instance()->createMethodInstance( object, methodName );
        objectCode += generate( method.get(), methodDependencies );
    }

    std::string dependencyCode;
    for ( auto className : methodDependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generateObjectMethodField( tempObject.get() );
        }
    }

    for ( auto className : dependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get(), false );
        }
    }

    return dependencyCode + objectCode;
}

std::string PythonGenerator::generateObjectMethodField( ObjectHandle* object )
{
    std::vector<std::string> dependencies;

    auto parentClassKeyword = findParentClass( object );
    dependencies.push_back( parentClassKeyword );

    std::string code;
    code += "class " + std::string( object->classKeyword() ) + "(" + parentClassKeyword + "):\n";
    if ( !object->classDocumentation().empty() )
    {
        code += "    \"\"\"" + object->classDocumentation() + "\"\"\"\n\n";
    }
    code += "    def __init__(self";

    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += ", " + field->keyword();
            auto childField      = dynamic_cast<ChildFieldHandle*>( field );
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );

            if ( childField )
            {
                dependencies.push_back( childClassKeyword( childField ) );
                code += ": " + childClassKeyword( childField ) + " = None";
            }
            else if ( childArrayField )
            {
                dependencies.push_back( childClassKeyword( childArrayField ) );
                code += ": " + childClassKeyword( childArrayField ) + " = []";
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
    code += "        super().__init__()\n";
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "        self.create_field(keyword=\"" + field->keyword() + "\",type=\"" + field->dataType() +
                    "\", value=" + field->keyword() + ")\n";
        }
    }
    code += "\n";

    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "    @property\n";
            code += "    def " + field->keyword() + "(self):\n";
            auto doc = field->capability<FieldDocumentationCapability>();
            if ( doc )
            {
                code += "        \"\"\"" + doc->documentation() + "\"\"\"\n\n";
            }
            code += "        return " + castFieldValue( field, "self.get(\"" + field->keyword() + "\")" ) + "\n\n";
            code += "    @" + field->keyword() + ".setter\n";
            code += "    def " + field->keyword() + "(self, value):\n";
            code += "        return self.set(\"" + field->keyword() + "\", value)\n\n";
        }
    }
    code += "    @classmethod\n";
    code += "    def copy(cls, object):\n";
    code += "        return cls(";

    bool first = true;
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            if ( !first ) code += ", ";

            code += castFieldValue( field, "object.get(\"" + field->keyword() + "\")" );
            first = false;
        }
    }
    code += ")\n";
    code += "\n\n";

    std::string dependencyCode;
    for ( auto className : dependencies )
    {
        if ( !isBuiltInClass( className ) && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generateObjectMethodField( tempObject.get() );
        }
    }
    return dependencyCode + code;
}

bool PythonGenerator::isBuiltInClass( const std::string& classKeyword ) const
{
    return classKeyword == "Object" || classKeyword == "Document";
}

std::string PythonGenerator::findParentClass( ObjectHandle* object ) const
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
        return childClassKeyword( childField ) + "(" + value + ")";
    }
    else if ( childArrayField )
    {
        return childClassKeyword( childArrayField ) + "(" + value + ")";
    }

    return value;
}

std::string PythonGenerator::dependency( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return childClassKeyword( childField );
    }
    else if ( childArrayField )
    {
        return childClassKeyword( childArrayField );
    }
    return "";
}

std::string PythonGenerator::pythonDataType( const caffa::FieldHandle* field ) const
{
    auto childField      = dynamic_cast<const ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<const ChildArrayFieldHandle*>( field );

    if ( childField )
    {
        return childClassKeyword( childField );
    }
    else if ( childArrayField )
    {
        return childClassKeyword( childArrayField );
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

std::string PythonGenerator::generate( caffa::ObjectMethod* method, std::vector<std::string>& dependencies )
{
    std::string code;

    auto fields = method->fields();

    code += "    def " + std::string( method->classKeyword() ) + "(self";

    if ( fields.size() > 0 )
    {
        for ( auto field : fields )
        {
            if ( field->keyword() == "uuid" ) continue;

            code += ", " + field->keyword();
            auto childField      = dynamic_cast<ChildFieldHandle*>( field );
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );

            if ( childField )
            {
                dependencies.push_back( childClassKeyword( childField ) );
                code += ": " + childClassKeyword( childField ) + " = " + childClassKeyword( childField ) + "()";
            }
            else if ( childArrayField )
            {
                dependencies.push_back( childClassKeyword( childArrayField ) );
                code += ": " + childClassKeyword( childArrayField ) + " = []";
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
    if ( !method->classDocumentation().empty() )
    {
        code += "        \"\"\"" + method->classDocumentation();
        std::string parametersCode;
        parametersCode += "        Parameters\n";
        parametersCode += "        ----------\n";
        size_t fieldCount = 0;
        for ( auto field : fields )
        {
            if ( field->keyword() != "uuid" )
            {
                auto doc = field->capability<FieldDocumentationCapability>();
                parametersCode += "        " + field->keyword() + " : " + pythonDataType( field ) + "\n";

                if ( doc )
                {
                    parametersCode += "            " + doc->documentation() + "\n";
                }
                fieldCount++;
            }
        }
        parametersCode += "        \"\"\"\n\n";
        if ( fieldCount > 0u )
            code += +"\n\n" + parametersCode;
        else
            code += "\"\"\"\n\n";
    }
    code += "        method = self.method(\"" + std::string( method->classKeyword() ) + "\")\n";
    for ( auto field : fields )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "        method." + field->keyword() + " = " + field->keyword() + "\n";
        }
    }

    auto resultObject = method->defaultResult();

    code += "        return " + std::string( resultObject->classKeyword() ) + ".copy(self.execute(method))\n";
    dependencies.push_back( std::string( resultObject->classKeyword() ) );

    code += "\n";
    return code;
}

bool pythonRegistered = caffa::rpc::CodeGeneratorFactory::instance()->registerCreator<caffa::rpc::PythonGenerator>(
    typeid( caffa::rpc::PythonGenerator ).hash_code() );
