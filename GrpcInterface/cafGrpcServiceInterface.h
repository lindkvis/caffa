// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
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

namespace caffa::rpc
{
class AbstractCallback;

/**
 * The base interface for all gRPC-services. Implement this to create a new service.
 */
class ServiceInterface
{
public:
    virtual std::vector<AbstractCallback*> createCallbacks() = 0;
    virtual ~ServiceInterface()                              = default;
};

typedef caffa::Factory<ServiceInterface, size_t> ServiceFactory;
} // namespace caffa::rpc
