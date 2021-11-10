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
grpc::Status ObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequest* request, RpcObject* reply )
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
grpc::Status ObjectService::GetDocuments( grpc::ServerContext* context, const NullMessage* request, RpcObjectList* reply )
{
    CAFFA_TRACE( "Got document request" );
    auto documents = ServerApplication::instance()->documents();
    CAFFA_TRACE( "Found document" );
    for ( auto document : documents )
    {
        RpcObject* newRpcObject = reply->add_objects();
        CAFFA_TRACE( "Copying document to gRPC data structure" );
        copyProjectObjectFromCafToRpc( document, newRpcObject );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, RpcObject* reply )
{
    const RpcObject& self = request->self_object();
    CAFFA_TRACE( "Execute method: " << request->method() );

    auto matchingObject = findCafObjectFromRpcObject( self );
    if ( matchingObject )
    {
        auto method = ObjectMethodFactory::instance()->createMethod( matchingObject, request->method() );
        if ( method )
        {
            copyResultOrParameterObjectFromRpcToCaf( &( request->params() ), method.get() );
            CAFFA_TRACE( "Method parameters copied. Now executing!" );
            auto [result, resultObject] = method->execute();
            if ( result )
            {
                if ( resultObject ) copyResultOrParameterObjectFromCafToRpc( resultObject.get(), reply );
                return grpc::Status::OK;
            }
            else
            {
                CAFFA_ERROR( "Failed to run execute!" );
                return grpc::Status( grpc::NOT_FOUND, std::string( "Method " ) + request->method() + " failed!" );
            }
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find Method" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find Object" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ListMethods( grpc::ServerContext* context, const RpcObject* self, RpcObjectList* reply )
{
    auto matchingObject = findCafObjectFromRpcObject( *self );
    CAFFA_ASSERT( matchingObject );
    CAFFA_TRACE( "Listing Object methods for " << matchingObject->classKeyword() );
    if ( matchingObject )
    {
        auto methodNames = ObjectMethodFactory::instance()->registeredMethodNames( matchingObject );
        CAFFA_TRACE( "Found " << methodNames.size() << " methods" );
        for ( auto methodName : methodNames )
        {
            CAFFA_TRACE( "Found method: " << methodName );
            auto       method          = ObjectMethodFactory::instance()->createMethod( matchingObject, methodName );
            RpcObject* newMethodObject = reply->add_objects();
            copyResultOrParameterObjectFromCafToRpc( method.get(), newMethodObject );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find Object" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* ObjectService::findCafObjectFromRpcObject( const RpcObject& rpcObject )
{
    auto [classKeyword, uuid] =
        caffa::ObjectJsonSerializer( true ).readClassKeywordAndUUIDFromObjectString( rpcObject.json() );
    return findCafObjectFromScriptNameAndUuid( classKeyword, uuid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* ObjectService::findCafObjectFromScriptNameAndUuid( const std::string& scriptClassName,
                                                                  const std::string& uuid )
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
        if ( testObject && testObject->uuid() == uuid )
        {
            matchingObject = testObject;
        }
    }
    return matchingObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool fieldIsScriptable( const caffa::FieldHandle* fieldHandle )
{
    return fieldHandle->capability<caffa::FieldScriptingCapability>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyProjectObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination )
{
    CAFFA_ASSERT( source && destination );
    CAFFA_ASSERT( !source->uuid().empty() );

    auto ioCapability = source->capability<caffa::ObjectIoCapability>();
    CAFFA_ASSERT( ioCapability );

    caffa::ObjectJsonSerializer serializer( false, DefaultObjectFactory::instance(), fieldIsScriptable );
    std::string                 jsonString = serializer.writeObjectToString( source );
    destination->set_json( jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyProjectObjectFromRpcToCaf( const RpcObject*      source,
                                                   caffa::ObjectHandle*  destination,
                                                   caffa::ObjectFactory* objectFactory )
{
    CAFFA_ASSERT( source && destination );

    caffa::ObjectJsonSerializer( true, objectFactory, fieldIsScriptable ).readObjectFromString( destination, source->json() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyResultOrParameterObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination )
{
    CAFFA_ASSERT( source && destination );

    auto jsonString = caffa::ObjectJsonSerializer( true ).writeObjectToString( source );
    destination->set_json( jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyResultOrParameterObjectFromRpcToCaf( const RpcObject* source, caffa::ObjectHandle* destination )
{
    CAFFA_ASSERT( source );
    caffa::ObjectJsonSerializer( true ).readObjectFromString( destination, source->json() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> ObjectService::createCafObjectFromRpc( const RpcObject*      source,
                                                                            caffa::ObjectFactory* objectFactory,
                                                                            bool                  copyDataValues )
{
    CAFFA_ASSERT( source );
    auto destination = caffa::ObjectJsonSerializer( copyDataValues, objectFactory, fieldIsScriptable )
                           .createObjectFromString( source->json() );

    return destination;
}

//--------------------------------------------------------------------------------------------------
/// Create a CAF object without requiring the fields to be scriptable.
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle>
    ObjectService::createResultOrParameterCafObjectFromRpc( const RpcObject*      source,
                                                            caffa::ObjectFactory* objectFactory,
                                                            bool                  copyDataValues )
{
    CAFFA_ASSERT( source );
    auto destination =
        caffa::ObjectJsonSerializer( copyDataValues, objectFactory, nullptr ).createObjectFromString( source->json() );

    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectMethod>
    ObjectService::createCafObjectMethodFromRpc( ObjectHandle*               self,
                                                 const RpcObject*            source,
                                                 caffa::ObjectMethodFactory* objectMethodFactory,
                                                 caffa::ObjectFactory*       objectFactory,
                                                 bool                        copyDataValues )
{
    CAFFA_ASSERT( self && source );
    if ( !self ) return nullptr;

    caffa::ObjectJsonSerializer serializer( copyDataValues, objectFactory, fieldIsScriptable );

    auto [classKeyword, uuid] = serializer.readClassKeywordAndUUIDFromObjectString( source->json() );

    std::unique_ptr<caffa::ObjectMethod> method = objectMethodFactory->createMethod( self, classKeyword );

    serializer.readObjectFromString( method.get(), source->json() );
    return method;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> ObjectService::createCallbacks()
{
    typedef ObjectService Self;
    return { new UnaryCallback<Self, DocumentRequest, RpcObject>( this, &Self::GetDocument, &Self::RequestGetDocument ),
             new UnaryCallback<Self, NullMessage, RpcObjectList>( this, &Self::GetDocuments, &Self::RequestGetDocuments ),
             new UnaryCallback<Self, MethodRequest, RpcObject>( this, &Self::ExecuteMethod, &Self::RequestExecuteMethod ),
             new UnaryCallback<Self, RpcObject, RpcObjectList>( this, &Self::ListMethods, &Self::RequestListMethods )

    };
}

} // namespace caffa::rpc
