//##################################################################################################
//
//   Caffa
//   Copyright (C) 2022- Kontur AS
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
#include "cafGrpcPythonGenerator.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafFieldHandle.h"
#include "cafFieldJsonCapability.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"
#include "cafObjectMethod.h"

using namespace caffa;
using namespace caffa::rpc;

PythonGenerator::~PythonGenerator()
{
}

std::string PythonGenerator::generate( std::list<std::unique_ptr<caffa::Document>>& documents )
{
    std::string code = "from caffa import Object\n\n\n";

    for ( auto& doc : documents )
    {
        code += generate( doc.get() );
    }
    return code;
}

std::string PythonGenerator::generate( ObjectHandle* object, bool objectMethodField )
{
    CAFFA_DEBUG( "Generating code for class " << object->classKeyword() );
    if ( objectMethodField )
    {
        return generateObjectMethodField( object );
    }

    std::string dependencyCode;

    auto parentClassKeyword = findParentClass( object );
    if ( parentClassKeyword != "Object" && !m_classesGenerated.count( parentClassKeyword ) )
    {
        m_classesGenerated.insert( parentClassKeyword );
        CAFFA_DEBUG( "Creating temp instance of " << parentClassKeyword );
        auto tempObject = caffa::DefaultObjectFactory::instance()->create( parentClassKeyword );
        dependencyCode += generate( tempObject.get(), objectMethodField );
    }

    std::string objectCode;
    objectCode += "class " + object->classKeyword() + "(" + parentClassKeyword + "):\n";
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
            objectCode += generate( field );
        }
    }

    auto methodNames = ObjectMethodFactory::instance()->registeredMethodNames( object );

    for ( auto methodName : methodNames )
    {
        auto method = ObjectMethodFactory::instance()->createMethod( object, methodName );
        objectCode += generate( method.get() );
    }

    for ( auto [className, objectMethodField] : m_classesToGenerate )
    {
        if ( className != "Object" && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get(), objectMethodField );
        }
    }

    return dependencyCode + objectCode;
}

std::string PythonGenerator::generateObjectMethodField( ObjectHandle* object )
{
    std::string dependencyCode;
    auto        parentClassKeyword = findParentClass( object );
    if ( parentClassKeyword != "Object" && !m_classesGenerated.count( parentClassKeyword ) )
    {
        m_classesGenerated.insert( parentClassKeyword );
        CAFFA_INFO( "Creating temp instance of " << parentClassKeyword );
        auto tempObject = caffa::DefaultObjectFactory::instance()->create( parentClassKeyword );
        dependencyCode += generate( tempObject.get(), true );
    }

    std::string code;
    code += "class " + object->classKeyword() + "(" + parentClassKeyword + "):\n";
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
                m_classesToGenerate.insert( std::make_pair( childField->childClassKeyword(), true ) );
                code += ": " + childField->childClassKeyword() + " = " + childField->childClassKeyword() + "()";
            }
            else if ( childArrayField )
            {
                m_classesToGenerate.insert( std::make_pair( childArrayField->childClassKeyword(), true ) );
                code += ": " + childArrayField->childClassKeyword() + " = []";
            }

            auto jsonCap = field->capability<FieldJsonCapability>();
            if ( jsonCap )
            {
                nlohmann::json json;
                jsonCap->writeToJson( json, JsonSerializer() );
                code += " = " + pythonValue( json["value"].dump() );
            }
        }
    }
    code += "):\n";
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "        self." + field->keyword() + " = " + field->keyword() + "\n";
        }
    }
    code += "\n";
    code += "    @classmethod\n";
    code += "    def copy(cls, object):\n";
    code += "        return cls(";

    bool first = true;
    for ( auto field : object->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            if ( !first ) code += ", ";
            code += "object.get(\"" + field->keyword() + "\")";
            first = false;
        }
    }
    code += ")\n";
    code += "\n\n";

    for ( auto [className, objectMethodField] : m_classesToGenerate )
    {
        if ( className != "Object" && !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            CAFFA_DEBUG( "Creating temp instance of " << className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            dependencyCode += generate( tempObject.get(), objectMethodField );
        }
    }

    return dependencyCode + code;
}

std::string PythonGenerator::findParentClass( ObjectHandle* object ) const
{
    auto inheritanceStack = object->classInheritanceStack();
    for ( size_t i = 1; i < inheritanceStack.size(); ++i )
    {
        auto object = DefaultObjectFactory::instance()->create( inheritanceStack[i] );
        if ( object )
        {
            return inheritanceStack[i];
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

std::string PythonGenerator::generate( FieldHandle* field )
{
    std::string code;

    auto childField      = dynamic_cast<ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();

    if ( !( scriptability && ( scriptability->isReadable() || scriptability->isWritable() ) ) ) return code;

    code += "    @property\n";
    code += "    def " + field->keyword() + "(self):\n";
    if ( scriptability->isReadable() )
    {
        if ( childField )
        {
            code += "        return " + childField->childClassKeyword() + "(self.get(\"" + field->keyword() + "\"))\n\n";
        }
        else if ( childArrayField )
        {
            code += "        return " + childArrayField->childClassKeyword() + "(self.get(\"" + field->keyword() +
                    "\"))\n\n";
        }
        else
        {
            code += "        return self.get(\"" + field->keyword() + "\")\n\n";
        }
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

    if ( childField )
    {
        m_classesToGenerate.insert( std::make_pair( childField->childClassKeyword(), false ) );
    }
    else if ( childArrayField )
    {
        m_classesToGenerate.insert( std::make_pair( childArrayField->childClassKeyword(), false ) );
    }

    return code;
}

std::string PythonGenerator::generate( caffa::ObjectMethod* method )
{
    std::string code;

    auto fields = method->fields();

    code += "    def " + method->classKeyword() + "(self";

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
                m_classesToGenerate.insert( std::make_pair( childField->childClassKeyword(), true ) );
                code += ": " + childField->childClassKeyword() + " = " + childField->childClassKeyword() + "()";
            }
            else if ( childArrayField )
            {
                m_classesToGenerate.insert( std::make_pair( childArrayField->childClassKeyword(), true ) );
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
    code += "        method = self.method(\"" + method->classKeyword() + "\")\n";
    for ( auto field : fields )
    {
        if ( field->keyword() != "uuid" )
        {
            code += "        method." + field->keyword() + " = " + field->keyword() + "\n";
        }
    }

    auto resultObject = method->defaultResult();

    code += "        return " + resultObject->classKeyword() + ".copy(self.execute(method))\n";
    m_classesToGenerate.insert( std::make_pair( resultObject->classKeyword(), true ) );

    code += "\n";
    return code;
}
