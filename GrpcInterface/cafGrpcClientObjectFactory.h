//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafObjectFactory.h"

#include "cafAssert.h"
#include "cafGrpcClient.h"
#include "cafGrpcDataFieldAccessor.h"

#include "cafPortableDataType.h"

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
    virtual std::unique_ptr<DataFieldAccessorInterface>
        create( Client* client, caffa::ObjectHandle* fieldOwner, const std::string& fieldName ) = 0;
};

template <typename DataType>
class AccessorCreator : public AccessorCreatorBase
{
public:
    std::unique_ptr<DataFieldAccessorInterface>
        create( Client* client, caffa::ObjectHandle* fieldOwner, const std::string& fieldName ) override
    {
        return std::make_unique<GrpcDataFieldAccessor<DataType>>( client, fieldOwner, fieldName );
    }
};

class GrpcClientObjectFactory : public ObjectFactory
{
public:
    static GrpcClientObjectFactory* instance();

    std::vector<std::string> classKeywords() const override;
    void                     setGrpcClient( Client* client );
    template <typename DataType>
    void registerBasicAccessorCreators()
    {
        registerAccessorCreator( PortableDataType<DataType>::name(), std::make_unique<AccessorCreator<DataType>>() );
        registerAccessorCreator( PortableDataType<std::vector<DataType>>::name(),
                                 std::make_unique<AccessorCreator<std::vector<DataType>>>() );
        registerAccessorCreator( PortableDataType<std::vector<std::vector<DataType>>>::name(),
                                 std::make_unique<AccessorCreator<std::vector<std::vector<DataType>>>>() );
    }

private:
    std::unique_ptr<ObjectHandle> doCreate( const std::string& classNameKeyword ) override;

    GrpcClientObjectFactory()
        : m_grpcClient( nullptr )
    {
        registerAllBasicAccessorCreators();
    }
    ~GrpcClientObjectFactory() override = default;

    void applyAccessorToField( caffa::ObjectHandle* objectHandle, caffa::FieldHandle* fieldHandle );

    void registerAccessorCreator( const std::string& dataType, std::unique_ptr<AccessorCreatorBase> creator );

    void registerAllBasicAccessorCreators();

private:
    Client* m_grpcClient;

    // Map to store factory
    std::map<std::string, std::unique_ptr<AccessorCreatorBase>> m_accessorCreatorMap;
};

} // namespace caffa::rpc
