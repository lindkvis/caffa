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
#include "cafRpcServer.h"
#include "cafSession.h"

#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcServerApplication.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafRpcObjectConversion.h"

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
        reply->set_json( createJsonSchemaFromProjectObject( document.get() ).dump() );
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
    CAFFA_TRACE( "Execute method: " << request->method_name() );

    auto session = GrpcServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    try
    {
        auto matchingObject = findCafObjectFromJsonObject( session.get(), self.json() );
        if ( matchingObject )
        {
            auto methodJson = nlohmann::json::parse( request->method_name() );
            auto method     = matchingObject->findMethod( methodJson["keyword"] );

            if ( method )
            {
                if ( method->type() == MethodHandle::Type::READ_WRITE && session->type() == Session::Type::OBSERVING )
                {
                    return grpc::Status( grpc::UNAUTHENTICATED, "Operation cannot be completed with observing sessions" );
                }

                auto result = method->execute( request->arguments().json() );
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

    auto matchingObject = findCafObjectFromJsonObject( session.get(), request->self_object().json() );

    if ( matchingObject )
    {
        CAFFA_TRACE( "Listing Object methods for " << matchingObject->classKeyword() );
        auto methods = matchingObject->methods();
        CAFFA_TRACE( "Found " << methods.size() << " methods" );
        for ( auto method : methods )
        {
            CAFFA_TRACE( "Found method: " << method->keyword() );
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
