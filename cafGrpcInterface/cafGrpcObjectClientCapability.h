//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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

#include "cafObjectCapability.h"

#include <cstdint>

namespace caf
{
namespace rpc
{
    class ObjectService;

    class ObjectClientCapability : public caf::ObjectCapability
    {
    public:
        ObjectClientCapability( uint64_t addressOnServer = 0u )
            : m_addressOnServer( addressOnServer )
        {
        }
        uint64_t addressOnServer() const { return m_addressOnServer; }

    private:
        friend class ObjectService;

        void setAddressOnServer( uint64_t address ) { m_addressOnServer = address; }

    private:
        uint64_t m_addressOnServer;
    };

} // namespace rpc
} // namespace caf
