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

#include <string>

#include <map>

namespace caf
{
class Object;

//==================================================================================================
/// Static register for object scriptability.
//==================================================================================================
class ObjectScriptingCapabilityRegister
{
public:
    static void        registerScriptClassNameAndComment( const std::string& classKeyword,
                                                          const std::string& scriptClassName,
                                                          const std::string& scriptClassComment );
    static std::string scriptClassNameFromClassKeyword( const std::string& classKeyword );
    static std::string classKeywordFromScriptClassName( const std::string& scriptClassName );
    static std::string scriptClassComment( const std::string& classKeyword );

    static bool isScriptable( const caf::Object* object );

private:
    static std::map<std::string, std::string> s_classKeywordToScriptClassName;
    static std::map<std::string, std::string> s_scriptClassNameToClassKeyword;
    static std::map<std::string, std::string> s_scriptClassComments;
};

} // namespace caf
