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

#include "ObjectService.grpc.pb.h"
#include "ObjectService.pb.h"

#include <map>
#include <string>
#include <thread>
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
class Serializer;
} // namespace caffa

namespace caffa::rpc
{
class FieldRequest;
class MethodRequest;
class NullMessage;

//==================================================================================================
//
// gRPC-service answering request searching for Objects in property tree
//
//==================================================================================================
class ObjectService final : public ObjectAccess::AsyncService, public ServiceInterface
{
public:
    grpc::Status GetDocument( grpc::ServerContext* context, const DocumentRequest* request, RpcObject* reply ) override;
    grpc::Status GetDocuments( grpc::ServerContext* context, const NullMessage* request, RpcObjectList* reply ) override;

    grpc::Status ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, RpcObject* reply ) override;
    grpc::Status ListMethods( grpc::ServerContext* context, const RpcObject* self, RpcObjectList* reply ) override;

    static caffa::Object* findCafObjectFromRpcObject( const RpcObject& rpcObject );
    static caffa::Object* findCafObjectFromFieldRequest( const FieldRequest& fieldRequest );
    static caffa::Object* findCafObjectFromScriptNameAndUuid( const std::string& scriptClassName, const std::string& uuid );

    static void copyProjectSelfReferenceFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination );
    static void copyProjectObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination );
    static void copyProjectObjectFromRpcToCaf( const RpcObject*      source,
                                               caffa::ObjectHandle*  destination,
                                               caffa::ObjectFactory* objectFactory );

    static void copyResultOrParameterObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination );
    static void copyResultOrParameterObjectFromRpcToCaf( const RpcObject* source, caffa::ObjectHandle* destination );

    static std::unique_ptr<caffa::ObjectHandle> createCafObjectFromRpc( const RpcObject*         source,
                                                                        const caffa::Serializer& serializer );

    static std::unique_ptr<caffa::ObjectMethod> createCafObjectMethodFromRpc( ObjectHandle*               self,
                                                                              const RpcObject*            source,
                                                                              caffa::ObjectMethodFactory* objectMethodFactory,
                                                                              caffa::ObjectFactory* objectFactory );

    std::vector<AbstractCallback*> createCallbacks() override;

private:
    static std::map<std::string, caffa::Object*> s_uuidCache;
    static std::mutex                            s_uuidCacheMutex;
};

} // namespace caffa::rpc
