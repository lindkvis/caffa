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
#include "cafFieldProxyAccessor.h"
#include "cafGrpcObjectClientCapability.h"
#include "cafGrpcServerApplication.h"
#include "cafObject.h"
#include "cafObjectMethod.h"
#include "cafObjectScriptingCapability.h"
#include "cafObjectScriptingCapabilityRegister.h"
#include "cafPdmDocument.h"
#include "cafPdmScriptIOMessages.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <vector>

namespace caf::rpc
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequest* request, Object* reply )
{
    std::cout << "Got document request" << std::endl;
    PdmDocument* document = ServerApplication::instance()->document( request->document_id() );
    std::cout << "Found document" << std::endl;
    if ( document )
    {
        std::cout << "Copying document to gRPC data structure" << std::endl;
        copyObjectFromCafToRpc( document, reply, true, false );
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Document not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, Object* reply )
{
    const Object& self           = request->self();
    auto          matchingObject = findCafObjectFromRpcObject( self );
    if ( matchingObject )
    {
        std::shared_ptr<ObjectMethod> method =
            ObjectMethodFactory::instance()->createMethod( matchingObject, request->method() );
        if ( method )
        {
            copyObjectFromRpcToCaf( &( request->params() ), method.get() );

            ObjectHandle* result = method->execute();
            if ( result )
            {
                copyObjectFromCafToRpc( result, reply, true, true );
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
    return findCafObjectFromScriptNameAndAddress( rpcObject.class_keyword(), rpcObject.address() );
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
void ObjectService::copyObjectFromCafToRpc( const caf::ObjectHandle* source,
                                            Object*                  destination,
                                            bool                     copyContent /* = true */,
                                            bool                     writeValues /* = true */ )
{
    CAF_ASSERT( source && destination );

    auto ioCapability = source->capability<caf::ObjectIoCapability>();
    CAF_ASSERT( ioCapability );

    destination->set_class_keyword( ioCapability->classKeyword() );
    auto clientCapability = source->capability<caf::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        destination->set_address( clientCapability->addressOnServer() );
    }
    else
    {
        destination->set_address( reinterpret_cast<uint64_t>( source ) );
    }

    if ( copyContent )
    {
        std::stringstream ss;
        caf::ObjectJsonCapability::writeFile( source, ss, true, writeValues );
        destination->set_json( ss.str() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyObjectFromRpcToCaf( const Object* source, caf::ObjectHandle* destination )
{
    CAF_ASSERT( source );

    auto              ioCapability = destination->capability<caf::ObjectIoCapability>();
    std::stringstream str( source->json() );
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
        caf::ObjectJsonCapability::readUnknownObjectFromString( source->json(), objectFactory, false ) );

    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> ObjectService::registerCallbacks()
{
    typedef ObjectService Self;
    return {
        new UnaryCallback<Self, DocumentRequest, Object>( this, &Self::GetDocument, &Self::RequestGetDocument ),
        new UnaryCallback<Self, MethodRequest, Object>( this, &Self::ExecuteMethod, &Self::RequestExecuteMethod ),

    };
}

} // namespace caf::rpc
