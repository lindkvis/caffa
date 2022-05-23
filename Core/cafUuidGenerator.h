//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2022- Kontur AS
//
//   This library may be used under the terms of the GNU Lesser General Public License as follows:
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

#include <memory>
#include <mutex>
#include <random>

namespace uuids
{
template <typename GeneratorT>
class basic_uuid_random_generator;
using uuid_random_generator = basic_uuid_random_generator<std::mt19937>;
} // namespace uuids

namespace caffa
{
class UuidGenerator
{
public:
    static std::string generate();

private:
    static std::unique_ptr<uuids::uuid_random_generator> s_uuidGenerator;
    static std::mutex                                    s_mutex;
};

} // namespace caffa