// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2024- Kontur AS (Changes from April 2021 and onwards)
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

#include "cafObjectFactory.h"

#include "cafAssert.h"
#include "cafRpcClient.h"
#include "cafRpcDataFieldAccessor.h"

#include "cafJsonDataType.h"

#include <map>
#include <string>
#include <vector>

namespace caffa::rpc
{
class Client;

class AccessorCreatorBase
{
public:
    AccessorCreatorBase() {}
    virtual ~AccessorCreatorBase() {}
    virtual std::unique_ptr<DataFieldAccessorInterface> create( Client* client, caffa::FieldHandle* fieldHandle ) = 0;
    virtual std::string                                 jsonDataType() const                                      = 0;
};

template <typename DataType>
class AccessorCreator : public AccessorCreatorBase
{
public:
    std::unique_ptr<DataFieldAccessorInterface> create( Client* client, caffa::FieldHandle* fieldHandle ) override
    {
        return std::make_unique<DataFieldAccessor<DataType>>( client, fieldHandle );
    }
    std::string jsonDataType() const override { return caffa::JsonDataType<DataType>::jsonType().dump(); }
};

/**
 * Client object factory which creates data fields and methods by reference.
 * If you wish to pass fields by value and still access remote methods, use the ClientPassByValueObjectFactory
 */

class ClientPassByRefObjectFactory : public ObjectFactory
{
public:
    static ClientPassByRefObjectFactory* instance();

    std::string name() const override { return "RPC Client Pass By Reference ObjectFactory"; }

    void setClient( Client* client );
    template <typename DataType>
    void registerBasicAccessorCreators()
    {
        registerAccessorCreator( JsonDataType<DataType>::jsonType().dump(), std::make_unique<AccessorCreator<DataType>>() );
        registerAccessorCreator( JsonDataType<std::vector<DataType>>::jsonType().dump(),
                                 std::make_unique<AccessorCreator<std::vector<DataType>>>() );
        registerAccessorCreator( JsonDataType<std::vector<std::vector<DataType>>>::jsonType().dump(),
                                 std::make_unique<AccessorCreator<std::vector<std::vector<DataType>>>>() );
    }

    std::list<std::string> supportedDataTypes() const;

private:
    std::shared_ptr<ObjectHandle> doCreate( const std::string_view& classKeyword ) override;

    ClientPassByRefObjectFactory()
        : m_client( nullptr )
    {
        registerAllBasicAccessorCreators();
    }
    ~ClientPassByRefObjectFactory() override = default;

    void applyAccessorToField( caffa::ObjectHandle* objectHandle, caffa::FieldHandle* fieldHandle );
    void applyNullAccessorToField( caffa::ObjectHandle* objectHandle, caffa::FieldHandle* fieldHandle );

    void applyAccessorToMethod( caffa::ObjectHandle* objectHandle, caffa::MethodHandle* methodHandle );

    void registerAccessorCreator( const std::string& dataType, std::unique_ptr<AccessorCreatorBase> creator );

    void registerAllBasicAccessorCreators();

private:
    Client* m_client;

    // Map to store factory
    std::map<std::string, std::unique_ptr<AccessorCreatorBase>> m_accessorCreatorMap;
};

} // namespace caffa::rpc
