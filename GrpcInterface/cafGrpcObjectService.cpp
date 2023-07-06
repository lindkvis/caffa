// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
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
#include "cafGrpcObjectService.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcServerApplication.h"
#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafSession.h"

#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcServerApplication.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"
#include "cafObjectCollector.h"

#include "FieldService.pb.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <vector>

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GrpcObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequest* request, RpcObject* reply )
{
    CAFFA_TRACE( "Got document request" );

    auto session = GrpcServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        CAFFA_ERROR( "Session '" << request->session().uuid() << "' is not valid" );
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    auto document = GrpcServerApplication::instance()->document( request->document_id(), session.get() );
    if ( document )
    {
        CAFFA_TRACE( "Found document with UUID: " << document->uuid() << " and will copy i tot gRPC data structure" );
        copyProjectObjectFromCafToRpc( document.get(), reply );
        return grpc::Status::OK;
    }
    CAFFA_WARNING( "Document not found '" + request->document_id() + "'" );
    return grpc::Status( grpc::NOT_FOUND, "Document not found: '" + request->document_id() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    GrpcObjectService::ListDocuments( grpc::ServerContext* context, const SessionMessage* request, DocumentList* reply )
{
    CAFFA_TRACE( "Got document request" );
    auto session = GrpcServerApplication::instance()->getExistingSession( request->uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->uuid() + "' is not valid" );
    }

    auto documents = GrpcServerApplication::instance()->documents( session.get() );
    CAFFA_TRACE( "Found " << documents.size() << " document" );
    for ( auto document : documents )
    {
        reply->add_document_id( document->id() );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GrpcObjectService::ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, RpcObject* reply )
{
    const RpcObject& self = request->self_object();
    CAFFA_TRACE( "Execute method: " << request->method().json() );

    auto session = GrpcServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    try
    {
        auto matchingObject = findCafObjectFromRpcObject( session.get(), self );
        if ( matchingObject )
        {
            auto methodJson = nlohmann::json::parse( request->method().json() );
            auto method     = matchingObject->findMethod( methodJson["keyword"] );

            if ( method )
            {
                if ( method->type() == MethodHandle::Type::READ_WRITE && session->type() == Session::Type::OBSERVING )
                {
                    return grpc::Status( grpc::UNAUTHENTICATED, "Operation cannot be completed with observing sessions" );
                }

                auto result = method->execute( request->method().json() );
                reply->set_json( result );

                CAFFA_TRACE( "Result JSON: " << reply->json() );

                return grpc::Status::OK;
            }
            return grpc::Status( grpc::NOT_FOUND, "Could not find Method" );
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find Object" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( e.what() );
        return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    GrpcObjectService::ListMethods( grpc::ServerContext* context, const ListMethodsRequest* request, RpcObjectList* reply )
{
    auto session = GrpcServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    auto matchingObject = findCafObjectFromRpcObject( session.get(), request->self_object() );

    if ( matchingObject )
    {
        CAFFA_TRACE( "Listing Object methods for " << matchingObject->classKeyword() );
        auto methods = matchingObject->methods();
        CAFFA_TRACE( "Found " << methods.size() << " methods" );
        for ( auto method : methods )
        {
            CAFFA_TRACE( "Found method: " << method->name() );
            RpcObject* newMethodObject = reply->add_objects();
            newMethodObject->set_json( method->schema() );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find Object" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* GrpcObjectService::findCafObjectFromRpcObject( const caffa::Session* session, const RpcObject& rpcObject )
{
    CAFFA_TRACE( "Looking for object from json: " << rpcObject.json() );
    auto [classKeyword, objectUuid] = caffa::JsonSerializer().readClassKeywordAndUUIDFromObjectString( rpcObject.json() );
    return findCafObjectFromScriptNameAndUuid( session, classKeyword, objectUuid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object* GrpcObjectService::findCafObjectFromScriptNameAndUuid( const caffa::Session* session,
                                                                      const std::string&    scriptClassName,
                                                                      const std::string&    objectUuid )
{
    CAFFA_TRACE( "Looking for caf object with class name '" << scriptClassName << "' and UUID '" << objectUuid << "'" );

    caffa::ObjectCollector<> collector(
        [scriptClassName]( const caffa::ObjectHandle* objectHandle ) -> bool
        { return ObjectHandle::matchesClassKeyword( scriptClassName, objectHandle->classInheritanceStack() ); } );

    for ( auto doc : GrpcServerApplication::instance()->documents( session ) )
    {
        doc->accept( &collector );
    }

    caffa::Object* matchingObject = nullptr;
    for ( ObjectHandle* testObjectHandle : collector.objects() )
    {
        caffa::Object* testObject = dynamic_cast<caffa::Object*>( testObjectHandle );
        CAFFA_TRACE( "Testing object with class name '" << testObject->classKeyword() << "' and UUID '"
                                                        << testObject->uuid() << "'" );

        if ( testObject && testObject->uuid() == objectUuid )
        {
            matchingObject = testObject;
        }
    }

    return matchingObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool fieldIsScriptReadable( const caffa::FieldHandle* fieldHandle )
{
    auto scriptability = fieldHandle->capability<caffa::FieldScriptingCapability>();
    return scriptability && scriptability->isReadable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool fieldIsScriptWritable( const caffa::FieldHandle* fieldHandle )
{
    auto scriptability = fieldHandle->capability<caffa::FieldScriptingCapability>();
    return scriptability && scriptability->isWritable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcObjectService::copyProjectSelfReferenceFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination )
{
    CAFFA_ASSERT( source && destination );
    CAFFA_ASSERT( !source->uuid().empty() );

    caffa::JsonSerializer serializer( DefaultObjectFactory::instance() );
    serializer.setFieldSelector( fieldIsScriptReadable );
    serializer.setWriteTypesAndValidators( false );
    serializer.setSerializeUuids( true );
    serializer.setSerializeDataTypes( false );

    std::string jsonString = serializer.writeObjectToString( source );
    CAFFA_TRACE( jsonString );
    destination->set_json( jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcObjectService::copyProjectObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination )
{
    CAFFA_ASSERT( source && destination );
    CAFFA_ASSERT( !source->uuid().empty() );

    caffa::JsonSerializer serializer( DefaultObjectFactory::instance() );
    serializer.setFieldSelector( fieldIsScriptReadable );
    serializer.setWriteTypesAndValidators( false );
    serializer.setSerializeUuids( true );

    std::string jsonString = serializer.writeObjectToString( source );
    destination->set_json( jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcObjectService::copyProjectObjectFromRpcToCaf( const RpcObject*      source,
                                                       caffa::ObjectHandle*  destination,
                                                       caffa::ObjectFactory* objectFactory )
{
    CAFFA_ASSERT( source && destination );

    caffa::JsonSerializer serializer( DefaultObjectFactory::instance() );
    serializer.setFieldSelector( fieldIsScriptWritable );
    serializer.setWriteTypesAndValidators( false );

    serializer.readObjectFromString( destination, source->json() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcObjectService::copyResultOrParameterObjectFromCafToRpc( const caffa::ObjectHandle* source, RpcObject* destination )
{
    CAFFA_ASSERT( source && destination );

    auto jsonString = caffa::JsonSerializer().writeObjectToString( source );
    destination->set_json( jsonString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcObjectService::copyResultOrParameterObjectFromRpcToCaf( const RpcObject* source, caffa::ObjectHandle* destination )
{
    CAFFA_ASSERT( source );
    caffa::JsonSerializer().readObjectFromString( destination, source->json() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<caffa::ObjectHandle> GrpcObjectService::createCafObjectFromRpc( const RpcObject*  source,
                                                                                const Serializer& serializer )
{
    CAFFA_ASSERT( source );

    auto destination = serializer.createObjectFromString( source->json() );
    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractGrpcCallback*> GrpcObjectService::createCallbacks()
{
    typedef GrpcObjectService Self;
    return { new GrpcServiceCallback<Self, DocumentRequest, RpcObject>( this, &Self::GetDocument, &Self::RequestGetDocument ),
             new GrpcServiceCallback<Self, SessionMessage, DocumentList>( this,
                                                                          &Self::ListDocuments,
                                                                          &Self::RequestListDocuments ),
             new GrpcServiceCallback<Self, MethodRequest, RpcObject>( this, &Self::ExecuteMethod, &Self::RequestExecuteMethod ),
             new GrpcServiceCallback<Self, ListMethodsRequest, RpcObjectList>( this, &Self::ListMethods, &Self::RequestListMethods )

    };
}

} // namespace caffa::rpc
