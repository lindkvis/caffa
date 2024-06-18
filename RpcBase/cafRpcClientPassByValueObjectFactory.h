// ##################################################################################################
//
//    Caffa - File copied and altered from cafClientPassByRefObjectFactory.h
//
//    Copyright (C) 2011- Ceetron AS (Changes up until April 2021)
//    Copyright (C) 2021- Kontur AS (Changes from April 2021 and onwards)
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

#include "cafRpcClientPassByRefObjectFactory.h"

namespace caffa::rpc
{
class Client;

/**
 * Adaptation of ClientPassByRefObjectFactory which assigns RPC *Method* Accessors but not field accessors.
 * Thus: fields are passed by value, but you can still call remote methods on them.
 */
class ClientPassByValueObjectFactory : public ObjectFactory
{
public:
    static ClientPassByValueObjectFactory* instance();

    std::string name() const override { return "Client Pass By Value ObjectFactory"; }

    void setClient( Client* client );

private:
    std::shared_ptr<ObjectHandle> doCreate( const std::string_view& classKeyword ) override;

    ClientPassByValueObjectFactory()
        : m_client( nullptr )
    {
    }
    ~ClientPassByValueObjectFactory() override = default;

    void applyAccessorToMethod( caffa::ObjectHandle* objectHandle, caffa::MethodHandle* methodHandle );

    void doApplyAccessors( caffa::not_null<caffa::ObjectHandle*> objectHandl ) override;

private:
    Client* m_client;
};

} // namespace caffa::rpc
