//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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
#include "cafGrpcObjectService.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcServerApplication.h"

#include "cafAbstractFieldScriptingCapability.h"
#include "cafField.h"
#include "cafGrpcObjectClientCapability.h"
#include "cafGrpcServerApplication.h"
#include "cafObject.h"
#include "cafObjectMethod.h"
#include "cafObjectScriptingCapability.h"
#include "cafObjectScriptingCapabilityRegister.h"
#include "cafPdmDocument.h"
#include "cafPdmScriptIOMessages.h"
#include "cafProxyValueField.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <vector>

namespace caf::rpc
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequestT* request, ObjectReply* reply )
{
    std::cout << "Got document request" << std::endl;
    auto docRequest = request->GetRoot();

    PdmDocument* document = ServerApplication::instance()->document( docRequest->document_id()->str() );
    std::cout << "Found document" << std::endl;
    if ( document )
    {
        std::cout << "Copying document to gRPC data structure" << std::endl;
        flatbuffers::grpc::MessageBuilder mb;

        auto object = copyObjectFromCafToRpc( mb, document, true, false );
        *reply      = mb.ReleaseMessage<Object>();
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Document not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteMethod( grpc::ServerContext* context, const MethodRequestT* request, ObjectReply* reply )
{
    auto methodRequest = request->GetRoot();
    auto self          = methodRequest->self();

    auto matchingObject = findCafObjectFromRpcObject( *self );
    if ( matchingObject )
    {
        std::shared_ptr<ObjectMethod> method =
            ObjectMethodFactory::instance()->createMethod( matchingObject, methodRequest->method()->str() );
        if ( method )
        {
            auto params = methodRequest->params();
            copyObjectFromRpcToCaf( params, method.get() );

            ObjectHandle* result = method->execute();
            if ( result )
            {
                flatbuffers::grpc::MessageBuilder mb;

                auto object = copyObjectFromCafToRpc( mb, result, true, false );
                *reply      = mb.ReleaseMessage<Object>();

                if ( !method->resultIsPersistent() )
                {
                    delete result;
                }
                return grpc::Status::OK;
            }
            else
            {
                if ( method->isNullptrValidResult() )
                {
                    return grpc::Status::OK;
                }

                return grpc::Status( grpc::NOT_FOUND, "No result returned from Method" );
            }
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find Method" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find PdmObject" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object* ObjectService::findCafObjectFromRpcObject( const Object& rpcObject )
{
    return findCafObjectFromScriptNameAndAddress( rpcObject.class_keyword()->str(), rpcObject.address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object* ObjectService::findCafObjectFromScriptNameAndAddress( const std::string& scriptClassName, uint64_t address )
{
    std::list<caf::ObjectHandle*> objectsOfCurrentClass;

    if ( caf::Application::instance()->hasCapability( AppCapability::GRPC_CLIENT ) )
    {
        objectsOfCurrentClass = GrpcClientObjectFactory::instance()->objectsWithClassKeyword( scriptClassName );
    }

    for ( auto doc : ServerApplication::instance()->documents() )
    {
        std::vector<caf::Object*> objects;
        doc->descendantsIncludingThisFromClassKeyword( scriptClassName, objects );
        for ( auto object : objects )
        {
            objectsOfCurrentClass.push_back( object );
        }
    }

    caf::Object* matchingObject = nullptr;
    for ( ObjectHandle* testObjectHandle : objectsOfCurrentClass )
    {
        caf::Object* testObject = dynamic_cast<caf::Object*>( testObjectHandle );
        if ( testObject && reinterpret_cast<uint64_t>( testObject ) == address )
        {
            matchingObject = testObject;
        }
    }
    return matchingObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
flatbuffers::Offset<Object> ObjectService::copyObjectFromCafToRpc( flatbuffers::grpc::MessageBuilder& builder,
                                                                   const caf::ObjectHandle*           source,
                                                                   bool copyContent /* = true */,
                                                                   bool writeValues /* = true */ )
{
    ObjectBuilder objectBuilder( builder );

    CAF_ASSERT( source );

    auto ioCapability = source->capability<caf::ObjectIoCapability>();
    CAF_ASSERT( ioCapability );

    objectBuilder.add_class_keyword( builder.CreateString( ioCapability->classKeyword() ) );
    auto clientCapability = source->capability<caf::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        objectBuilder.add_address( clientCapability->addressOnServer() );
    }
    else
    {
        objectBuilder.add_address( reinterpret_cast<uint64_t>( source ) );
    }

    if ( copyContent )
    {
        std::stringstream ss;
        caf::ObjectJsonCapability::writeFile( source, ss, true, writeValues );
        objectBuilder.add_json( builder.CreateString( ss.str() ) );
    }
    return objectBuilder.Finish();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyObjectFromRpcToCaf( const Object* source, caf::ObjectHandle* destination )
{
    CAF_ASSERT( source );

    auto              ioCapability = destination->capability<caf::ObjectIoCapability>();
    std::stringstream str( source->json()->data() );
    ioCapability->readFile( str );

    auto clientCapability = destination->capability<caf::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        clientCapability->setAddressOnServer( source->address() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> ObjectService::createCafObjectFromRpc( const Object*       source,
                                                                          caf::ObjectFactory* objectFactory )
{
    CAF_ASSERT( source );
    std::unique_ptr<caf::ObjectHandle> destination(
        caf::ObjectJsonCapability::readUnknownObjectFromString( source->json()->str(), objectFactory, false ) );

    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> ObjectService::registerCallbacks()
{
    typedef ObjectService Self;
    return {
        new UnaryCallback<Self, DocumentRequestT, ObjectReply>( this, &Self::GetDocument ),
        new UnaryCallback<Self, MethodRequestT, ObjectReply>( this, &Self::ExecuteMethod ),

    };
}

} // namespace caf::rpc
