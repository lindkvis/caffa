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

#include "cafGrpcCodeGenerator.h"

#include <list>
#include <map>
#include <set>
#include <string>

namespace caffa
{
class FieldHandle;
class ObjectHandle;
class ObjectMethod;

namespace rpc
{
    class MarkdownGenerator : public CodeGenerator
    {
    public:
        ~MarkdownGenerator() override;
        std::string name() const override;

        std::string generate( std::list<std::shared_ptr<caffa::Document>>& documents ) override;
        std::string generate( std::shared_ptr<caffa::ObjectHandle> object,
                              bool                                 objectMethodResultOrParameter = false ) override;
        std::string generate( caffa::FieldHandle* field, std::vector<std::string>& dependencies ) override;
        std::string generate( caffa::ObjectMethod* method, std::vector<std::string>& dependencies ) override;

    private:
        std::string generateObjectMethodField( std::shared_ptr<caffa::ObjectHandle> object );
        std::string findParentClass( std::shared_ptr<caffa::ObjectHandle> object ) const;
        std::string dependency( const caffa::FieldHandle* field ) const;
        std::string docDataType( const caffa::FieldHandle* field ) const;

        std::set<std::string> m_classesGenerated;
    };
} // namespace rpc
} // namespace caffa