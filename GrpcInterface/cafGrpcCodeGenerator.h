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

#include <list>
#include <memory>
#include <string>

namespace caffa
{
class Document;
class FieldHandle;
class ObjectHandle;
class ObjectMethod;

namespace rpc
{
    class CodeGenerator
    {
    public:
        virtual ~CodeGenerator() = default;

        virtual std::string generate( std::list<std::unique_ptr<caffa::Document>>& documents )      = 0;
        virtual std::string generate( caffa::ObjectHandle* object, bool objectMethodField = false ) = 0;
        virtual std::string generate( caffa::FieldHandle* field )                                   = 0;
        virtual std::string generate( caffa::ObjectMethod* method )                                 = 0;
    };
} // namespace rpc
} // namespace caffa