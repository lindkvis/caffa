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

#include "cafGrpcServiceInterface.h"

#include "FieldService.grpc.pb.h"
#include "FieldService.pb.h"

#include <map>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace caffa
{
class FieldHandle;
class ObjectHandle;
} // namespace caffa

namespace caffa::rpc
{
class AbstractCallback;
class GenericValue;
class FieldRequest;
class NullMessage;
class SetterRequest;

//==================================================================================================
//
// gRPC-service answering request searching for Objects in property tree
//
//==================================================================================================
class FieldService final : public FieldAccess::AsyncService, public ServiceInterface
{
public:
    grpc::Status GetValue( grpc::ServerContext* context, const FieldRequest* request, GenericValue* reply ) override;
    grpc::Status SetValue( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply ) override;

    grpc::Status ClearChildObjects( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply ) override;
    grpc::Status RemoveChildObject( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply ) override;
    grpc::Status InsertChildObject( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply ) override;
    std::vector<AbstractCallback*> createCallbacks() override;

    static caffa::FieldHandle* scriptableFieldFromKeyword( const caffa::ObjectHandle* fieldOwner,
                                                           const std::string&         keyword );

private:
    static std::map<const caffa::ObjectHandle*, std::map<std::string, caffa::FieldHandle*>> s_fieldCache;
    static std::mutex                                                                       s_fieldCacheMutex;
};

} // namespace caffa::rpc
