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
#include "cafFieldScriptingCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectMethod.h"

using namespace caffa;
using namespace caffa::rpc;

PythonGenerator::~PythonGenerator()
{
}

std::string PythonGenerator::generate( std::list<std::unique_ptr<caffa::Document>>& documents )
{
    std::string code = "import caffa\n\n";

    for ( auto& doc : documents )
    {
        code += generate( doc.get() );
    }
    return code;
}

std::string PythonGenerator::generate( ObjectHandle* object, const std::string& parentClassKeyword )
{
    std::string code = "class " + object->classKeyword() + "(" + parentClassKeyword + "):\n ";
    code += "    def __init__(self, object):\n";
    code += "        self._json_object = object._json_object\n";
    code += "        self._session_uuid = object._session_uuid\n";
    code += "        self._object_cache = object._object_cache\n";
    code += "        self._channel = object._channel\n";
    code += "        self._field_stub = object._field_stub\n";
    code += "        self._object_stub = object._object_stub\n\n";

    for ( auto field : object->fields() )
    {
        if ( field->capability<caffa::FieldScriptingCapability>() != nullptr ) code += generate( field );
    }

    auto methodNames = ObjectMethodFactory::instance()->registeredMethodNames( object );

    for ( auto methodName : methodNames )
    {
        auto method = ObjectMethodFactory::instance()->createMethod( object, methodName );
        code += generate( method.get() );
    }

    for ( auto className : m_classesToGenerate )
    {
        if ( !m_classesGenerated.count( className ) )
        {
            m_classesGenerated.insert( className );
            auto tempObject = caffa::DefaultObjectFactory::instance()->create( className );
            code += generate( tempObject.get(), object->classKeyword() );
        }
    }

    return code;
}

std::string PythonGenerator::generate( FieldHandle* field )
{
    std::string code;

    auto childField      = dynamic_cast<ChildFieldHandle*>( field );
    auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );

    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( scriptability->isReadable() )
    {
        code += "    @property\n";
        code += "    def " + field->keyword() + "(self):\n";
        if ( childField )
        {
            code += "        return " + childField->childClassKeyword() + "(self.get(" + field->keyword() + "))\n\n";
        }
        else if ( childArrayField )
        {
            code += "        return " + childArrayField->childClassKeyword() + "(self.get(" + field->keyword() + "))\n\n";
        }
        else
        {
            code += "        return self.get(" + field->keyword() + ")\n\n";
        }
    }
    if ( scriptability->isWritable() )
    {
        code += "    @" + field->keyword() + ".setter\n";
        code += "    def " + field->keyword() + "(self, value):\n";
        code += "        return self.set(" + field->keyword() + ", value)\n\n";
    }

    if ( childField )
    {
        m_classesToGenerate.insert( childField->childClassKeyword() );
    }
    else if ( childArrayField )
    {
        m_classesToGenerate.insert( childArrayField->childClassKeyword() );
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
            if ( field->keyword() != "uuid" ) code += ", " + field->keyword();
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
    code += "        return self.execute(method)\n";

    code += "\n";
    return code;
}
