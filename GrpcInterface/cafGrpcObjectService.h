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

#include "cafGrpcCallbacks.h"
#include "cafGrpcServiceInterface.h"
#include "cafVariant.h"

#include "ObjectService.grpc.pb.h"
#include "ObjectService.pb.h"

#include <string>
#include <vector>

namespace caffa
{
class ChildFieldHandle;
class FieldHandle;
class Object;
class ObjectFactory;
class ObjectHandle;
class ObjectMethod;
class ObjectMethodFactory;
class ProxyFieldHandle;
class ValueField;
} // namespace caffa

namespace caffa::rpc
{
class MethodRequest;

//==================================================================================================
//
// gRPC-service answering request searching for Objects in property tree
//
//==================================================================================================
class ObjectService final : public ObjectAccess::AsyncService, public ServiceInterface
{
public:
    grpc::Status GetDocument( grpc::ServerContext* context, const DocumentRequest* request, Object* reply );

    grpc::Status ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, Object* reply ) override;
    grpc::Status ListMethods( grpc::ServerContext* context, const Object* self, ObjectList* reply ) override;

    static caffa::Object* findCafObjectFromRpcObject( const Object& rpcObject );
    static caffa::Object* findCafObjectFromScriptNameAndUuid( const std::string& scriptClassName, const std::string& uuid );

    static void copyProjectObjectFromCafToRpc( const caffa::ObjectHandle* source, Object* destination );
    static void copyProjectObjectFromRpcToCaf( const Object* source, caffa::ObjectHandle* destination );

    static void copyResultOrParameterObjectFromCafToRpc( const caffa::ObjectHandle* source, Object* destination );
    static void copyResultOrParameterObjectFromRpcToCaf( const Object* source, caffa::ObjectHandle* destination );

    static std::unique_ptr<caffa::ObjectHandle>
        createCafObjectFromRpc( const Object* source, caffa::ObjectFactory* objectFactory, bool copyDataValues );

    static std::unique_ptr<caffa::ObjectMethod> createCafObjectMethodFromRpc( ObjectHandle*               self,
                                                                              const Object*               source,
                                                                              caffa::ObjectMethodFactory* objectMethodFactory,
                                                                              caffa::ObjectFactory*       objectFactory,
                                                                              bool copyDataValues );

    std::vector<AbstractCallback*> createCallbacks() override;
};

} // namespace caffa::rpc
