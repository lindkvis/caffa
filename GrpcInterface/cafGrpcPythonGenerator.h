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
#pragma once

#include "cafGrpcCodeGenerator.h"

#include <list>
#include <set>
#include <string>

namespace caffa
{
class FieldHandle;
class ObjectHandle;
class ObjectMethod;

namespace rpc
{
    class PythonGenerator : public CodeGenerator
    {
    public:
        ~PythonGenerator() override;

        std::string generate( std::list<std::unique_ptr<caffa::Document>>& documents ) override;
        std::string generate( caffa::ObjectHandle* object, const std::string& parentClassKeyword = "Object" ) override;
        std::string generate( caffa::FieldHandle* field ) override;
        std::string generate( caffa::ObjectMethod* method ) override;

    private:
        std::set<std::string> m_classesToGenerate;
        std::set<std::string> m_classesGenerated;
    };
} // namespace rpc
} // namespace caffa