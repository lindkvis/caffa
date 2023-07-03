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
#pragma once

#include "cafFactory.h"

#include <list>
#include <memory>
#include <string>

namespace caffa
{
class Document;
class FieldHandle;
class ObjectHandle;
class MethodHandle;

namespace rpc
{
    class CodeGenerator
    {
    public:
        virtual ~CodeGenerator() = default;

        virtual std::string name() const = 0;

        virtual std::string generate( std::list<std::shared_ptr<caffa::Document>>& documents )      = 0;
        virtual std::string generate( const caffa::ObjectHandle* object, bool passByValue = false ) = 0;
        virtual std::string
            generate( const caffa::FieldHandle* field, bool passByValue, std::vector<std::string>& dependencies ) = 0;
        virtual std::string generate( const caffa::MethodHandle* method, std::vector<std::string>& dependencies ) = 0;
    };

    typedef caffa::Factory<CodeGenerator, size_t> CodeGeneratorFactory;
} // namespace rpc
} // namespace caffa