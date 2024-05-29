// ##################################################################################################
//
//    Copyright (C) 2023- Kontur AS
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
// ##################################################################################################
#pragma once

#include "cafObjectHandle.h"

#include <nlohmann/json.hpp>

#include <memory>

namespace caffa
{

void to_json( nlohmann::json& jsonValue, const ObjectHandle& object );
void from_json( const nlohmann::json& jsonValue, ObjectHandle& object );

void to_json( nlohmann::json& jsonValue, std::shared_ptr<const ObjectHandle> object );
void from_json( const nlohmann::json& jsonValue, std::shared_ptr<ObjectHandle> object );

} // namespace caffa