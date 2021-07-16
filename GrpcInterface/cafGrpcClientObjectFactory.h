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

#include <map>
#include <string>
#include <vector>

namespace caffa::rpc
{
//==================================================================================================
/// "Private" class for implementation of a factory for ObjectBase derived objects
/// Every Object must register with this factory to be readable
/// This class can be considered private in the Pdm system
//==================================================================================================

class GrpcClientObjectFactory : public ObjectFactory
{
public:
    static GrpcClientObjectFactory* instance();

    std::vector<std::string> classKeywords() const override;
    void                     setGrpcClient( Client* client );

private:
    std::unique_ptr<ObjectHandle> doCreate( const std::string& classNameKeyword ) override;

    GrpcClientObjectFactory()
        : m_grpcClient( nullptr )
    {
    }
    ~GrpcClientObjectFactory() override = default;

    void applyAccessorToField( caffa::ObjectHandle* objectHandle, caffa::FieldHandle* fieldHandle );

private:
    Client* m_grpcClient;
};

} // namespace caffa::rpc
