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
        copyProjectObjectFromCafToRpc( document, reply );
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
            copyResultOrParameterObjectFromRpcToCaf( &( request->params() ), method.get() );

            ObjectHandle* result = method->execute();
            if ( result )
            {
                copyResultOrParameterObjectFromCafToRpc( result, reply );
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
    auto jsonObject   = nlohmann::json::parse( rpcObject.json() );
    auto classKeyword = jsonObject["classKeyword"].get<std::string>();
    CAFFA_ASSERT( jsonObject.contains( "serverAddress" ) && jsonObject["serverAddress"].is_number_integer() );
    auto address = jsonObject["serverAddress"].get<uint64_t>();
    return findCafObjectFromScriptNameAndAddress( classKeyword, address );
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
void ObjectService::copyProjectObjectFromCafToRpc( const caffa::ObjectHandle* source, Object* destination )
{
    CAFFA_ASSERT( source && destination );

    auto ioCapability = source->capability<caffa::ObjectIoCapability>();
    CAFFA_ASSERT( ioCapability );
    auto clientCapability = source->capability<caffa::rpc::ObjectClientCapability>();

    std::stringstream ss;
    // Don't write the server address if this is client to server
    caffa::ObjectJsonCapability::writeFile( source, ss, clientCapability == nullptr, false );

    nlohmann::json jsonObject = nlohmann::json::parse( ss.str() );
    if ( clientCapability )
    {
        jsonObject["serverAddress"] = clientCapability->addressOnServer();
    }
    destination->set_json( jsonObject.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyProjectObjectFromRpcToCaf( const Object* source, caffa::ObjectHandle* destination )
{
    CAFFA_ASSERT( source );

    auto clientCapability = destination->capability<caffa::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        auto jsonObject = nlohmann::json::parse( source->json() );
        clientCapability->setAddressOnServer( jsonObject["serverAddress"].get<uint64_t>() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyResultOrParameterObjectFromCafToRpc( const caffa::ObjectHandle* source, Object* destination )
{
    CAFFA_ASSERT( source && destination );

    std::stringstream ss;
    caffa::ObjectJsonCapability::writeFile( source, ss, true, true );

    nlohmann::json jsonObject = nlohmann::json::parse( ss.str() );
    destination->set_json( jsonObject.dump() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyResultOrParameterObjectFromRpcToCaf( const Object* source, caffa::ObjectHandle* destination )
{
    CAFFA_ASSERT( source );

    auto clientCapability = destination->capability<caffa::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        auto jsonObject = nlohmann::json::parse( source->json() );
        clientCapability->setAddressOnServer( jsonObject["serverAddress"].get<uint64_t>() );
    }

    CAFFA_DEBUG( "Copying rpc object: " << source->json() );

    std::stringstream str( source->json() );
    auto              ioCapability = destination->capability<caffa::ObjectIoCapability>();
    ioCapability->readFile( str );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle>
    ObjectService::createCafObjectFromRpc( const Object* source, caffa::ObjectFactory* objectFactory, bool copyDataValues )
{
    CAFFA_ASSERT( source );
    std::unique_ptr<caffa::ObjectHandle> destination(
        caffa::ObjectJsonCapability::readUnknownObjectFromString( source->json(), objectFactory, copyDataValues ) );

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
