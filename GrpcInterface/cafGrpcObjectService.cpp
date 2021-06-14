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

#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcObjectClientCapability.h"
#include "cafGrpcServerApplication.h"
#include "cafObject.h"
#include "cafObjectMethod.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <vector>

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequest* request, Object* reply )
{
    CAFFA_TRACE( "Got document request" );
    Document* document = ServerApplication::instance()->document( request->document_id() );
    CAFFA_TRACE( "Found document" );
    if ( document )
    {
        CAFFA_TRACE( "Copying document to gRPC data structure" );
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
    return grpc::Status( grpc::NOT_FOUND, "Could not find Object" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* ObjectService::findCafObjectFromRpcObject( const Object& rpcObject )
{
    return findCafObjectFromScriptNameAndAddress( rpcObject.class_keyword(), rpcObject.address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* ObjectService::findCafObjectFromScriptNameAndAddress( const std::string& scriptClassName, uint64_t address )
{
    std::list<caffa::ObjectHandle*> objectsOfCurrentClass;

    if ( caffa::Application::instance()->hasCapability( AppCapability::GRPC_CLIENT ) )
    {
        objectsOfCurrentClass = GrpcClientObjectFactory::instance()->objectsWithClassKeyword( scriptClassName );
    }

    for ( auto doc : ServerApplication::instance()->documents() )
    {
        std::list<caffa::ObjectHandle*> objects = doc->matchingDescendants(
            [scriptClassName]( const caffa::ObjectHandle* objectHandle ) -> bool
            {
                auto ioCapability = objectHandle->capability<caffa::ObjectIoCapability>();
                return ioCapability ? ioCapability->classKeyword() == scriptClassName : false;
            } );

        for ( auto object : objects )
        {
            objectsOfCurrentClass.push_back( object );
        }
        if ( doc->classKeyword() == scriptClassName )
        {
            objectsOfCurrentClass.push_front( doc );
        }
    }

    caffa::Object* matchingObject = nullptr;
    for ( ObjectHandle* testObjectHandle : objectsOfCurrentClass )
    {
        caffa::Object* testObject = dynamic_cast<caffa::Object*>( testObjectHandle );
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
void ObjectService::copyObjectFromCafToRpc( const caffa::ObjectHandle* source,
                                            Object*                    destination,
                                            bool                       copyContent /* = true */,
                                            bool                       writeValues /* = true */ )
{
    CAFFA_ASSERT( source && destination );

    auto ioCapability = source->capability<caffa::ObjectIoCapability>();
    CAFFA_ASSERT( ioCapability );

    destination->set_class_keyword( ioCapability->classKeyword() );
    auto clientCapability = source->capability<caffa::rpc::ObjectClientCapability>();
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
        caffa::ObjectJsonCapability::writeFile( source, ss, true, writeValues );
        destination->set_json( ss.str() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyObjectFromRpcToCaf( const Object* source, caffa::ObjectHandle* destination )
{
    CAFFA_ASSERT( source );

    auto              ioCapability = destination->capability<caffa::ObjectIoCapability>();
    std::stringstream str( source->json() );
    ioCapability->readFile( str );

    auto clientCapability = destination->capability<caffa::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        clientCapability->setAddressOnServer( source->address() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> ObjectService::createCafObjectFromRpc( const Object*         source,
                                                                            caffa::ObjectFactory* objectFactory )
{
    CAFFA_ASSERT( source );
    std::unique_ptr<caffa::ObjectHandle> destination(
        caffa::ObjectJsonCapability::readUnknownObjectFromString( source->json(), objectFactory, false ) );

    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> ObjectService::createCallbacks()
{
    typedef ObjectService Self;
    return {
        new UnaryCallback<Self, DocumentRequest, Object>( this, &Self::GetDocument, &Self::RequestGetDocument ),
        new UnaryCallback<Self, MethodRequest, Object>( this, &Self::ExecuteMethod, &Self::RequestExecuteMethod ),

    };
}

} // namespace caffa::rpc
