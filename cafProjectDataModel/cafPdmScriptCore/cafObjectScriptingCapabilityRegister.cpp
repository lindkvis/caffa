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
#include "cafObjectScriptingCapabilityRegister.h"

#include "cafObject.h"

using namespace caf;

std::map<std::string, std::string> ObjectScriptingCapabilityRegister::s_classKeywordToScriptClassName;
std::map<std::string, std::string> ObjectScriptingCapabilityRegister::s_scriptClassNameToClassKeyword;
std::map<std::string, std::string> ObjectScriptingCapabilityRegister::s_scriptClassComments;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( const std::string& classKeyword,
                                                                           const std::string& scriptClassName,
                                                                           const std::string& scriptClassComment )
{
    s_classKeywordToScriptClassName[classKeyword]    = scriptClassName;
    s_scriptClassNameToClassKeyword[scriptClassName] = classKeyword;
    s_scriptClassComments[classKeyword]              = scriptClassComment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( const std::string& classKeyword )
{
    auto it = s_classKeywordToScriptClassName.find( classKeyword );
    if ( it != s_classKeywordToScriptClassName.end() )
    {
        return it->second;
    }
    return classKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( const std::string& scriptClassName )
{
    auto it = s_scriptClassNameToClassKeyword.find( scriptClassName );
    if ( it != s_scriptClassNameToClassKeyword.end() )
    {
        return it->second;
    }
    return scriptClassName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectScriptingCapabilityRegister::scriptClassComment( const std::string& classKeyword )
{
    auto it = s_scriptClassComments.find( classKeyword );
    if ( it != s_scriptClassComments.end() )
    {
        return it->second;
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectScriptingCapabilityRegister::isScriptable( const caf::Object* object )
{
    return s_classKeywordToScriptClassName.find( object->classKeyword() ) != s_classKeywordToScriptClassName.end();
}
