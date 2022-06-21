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
#include "cafGrpcClient.h"

#include "AppInfo.grpc.pb.h"
#include "ObjectService.grpc.pb.h"

#include "cafDefaultObjectFactory.h"
#include "cafGrpcApplication.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcException.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcObjectService.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>
#include <memory>

namespace caffa::rpc
{
class ClientImpl
{
public:
    ClientImpl( const std::string& hostname,
                int                port,
                const std::string& clientCertFile,
                const std::string& clientKeyFile,
                const std::string& caCertFile )
    {
        // Created new server
        if ( !caCertFile.empty() )
        {
            CAFFA_DEBUG( "Connecting to " << hostname << " using client cert: " << clientCertFile
                                          << " and key: " << clientKeyFile << " plus CA cert: " << caCertFile );

            std::string clientCert = Application::readKeyAndCertificate( clientCertFile );
            std::string clientKey  = Application::readKeyAndCertificate( clientKeyFile );
            std::string caCert     = Application::readKeyAndCertificate( caCertFile );

            grpc::SslCredentialsOptions ssl_opts;
            ssl_opts.pem_cert_chain  = clientCert;
            ssl_opts.pem_private_key = clientKey;
            ssl_opts.pem_root_certs  = caCert;

            auto ssl_creds = grpc::SslCredentials( ssl_opts );
            m_channel      = grpc::CreateChannel( hostname + ":" + std::to_string( port ), ssl_creds );
        }
        else
        {
            m_channel = grpc::CreateChannel( hostname + ":" + std::to_string( port ), grpc::InsecureChannelCredentials() );
        }
        CAFFA_DEBUG( "Created channel for " << hostname << ":" << port );
        m_appInfoStub = App::NewStub( m_channel );
        m_objectStub  = ObjectAccess::NewStub( m_channel );
        m_fieldStub   = FieldAccess::NewStub( m_channel );
        CAFFA_TRACE( "Created stubs" );
        createSession();
    }
    ~ClientImpl()
    {
        //
        CAFFA_DEBUG( "Destroying client" );
        destroySession();
    }

    const std::string& sessionUuid() const { return m_sessionUuid; }

    void createSession()
    {
        caffa::rpc::SessionMessage session;
        grpc::ClientContext        context;
        NullMessage                nullarg;

        auto status = m_appInfoStub->CreateSession( &context, nullarg, &session );
        if ( !status.ok() )
        {
            CAFFA_ERROR( status.error_message() );
            throw Exception( status );
        }

        m_sessionUuid = session.uuid();
        CAFFA_DEBUG( "Created session " << m_sessionUuid );
    }

    void sendKeepAlive()
    {
        CAFFA_DEBUG( "Keeping session alive" << m_sessionUuid );

        caffa::rpc::SessionMessage session;
        session.set_uuid( m_sessionUuid );
        grpc::ClientContext context;
        NullMessage         nullreply;

        auto status = m_appInfoStub->KeepSessionAlive( &context, session, &nullreply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( status.error_message() );
            throw Exception( status );
        }
    }

    void destroySession()
    {
        if ( !m_sessionUuid.empty() )
        {
            CAFFA_DEBUG( "Destroying session " << m_sessionUuid );

            caffa::rpc::SessionMessage session;
            session.set_uuid( m_sessionUuid );
            grpc::ClientContext context;
            NullMessage         nullreply;

            auto status = m_appInfoStub->DestroySession( &context, session, &nullreply );
            if ( !status.ok() )
            {
                CAFFA_ERROR( status.error_message() );
                throw Exception( status );
            }

            m_sessionUuid = "";
            CAFFA_TRACE( "Session destroyed" );
        }
    }

    caffa::AppInfo appInfo() const
    {
        CAFFA_TRACE( "Trying to get app info" );
        caffa::rpc::AppInfoReply reply;
        grpc::ClientContext      context;
        NullMessage              nullarg;
        auto                     status = m_appInfoStub->GetAppInfo( &context, nullarg, &reply );
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        caffa::AppInfo appInfo = { reply.name(),
                                   reply.major_version(),
                                   reply.minor_version(),
                                   reply.patch_version(),
                                   reply.type() };
        return appInfo;
    }

    std::unique_ptr<caffa::ObjectHandle> document( const std::string& documentId ) const
    {
        std::unique_ptr<caffa::ObjectHandle> document;

        grpc::ClientContext         context;
        caffa::rpc::DocumentRequest request;
        request.set_document_id( documentId );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        request.set_allocated_session( session.release() );

        caffa::rpc::RpcObject objectReply;
        CAFFA_TRACE( "Calling GetDocument()" );
        auto status = m_objectStub->GetDocument( &context, request, &objectReply );
        if ( status.ok() )
        {
            CAFFA_TRACE( "Got document" );
            caffa::JsonSerializer serializer( caffa::rpc::GrpcClientObjectFactory::instance() );
            serializer.setSerializeDataValues( false );
            document = caffa::rpc::ObjectService::createCafObjectFromRpc( &objectReply, serializer );
            CAFFA_TRACE( "Document completed with UUID " << document->uuid() );
        }
        else
        {
            CAFFA_ERROR( "Failed to get document with server error message: " + status.error_message() );
            throw Exception( status );
        }
        return document;
    }

    std::vector<std::unique_ptr<caffa::ObjectHandle>> documents() const
    {
        std::vector<std::unique_ptr<caffa::ObjectHandle>> documents;

        grpc::ClientContext        context;
        caffa::rpc::SessionMessage request;
        request.set_uuid( m_sessionUuid );

        caffa::rpc::DocumentList objectListReply;
        CAFFA_TRACE( "Calling ListDocuments()" );
        auto status = m_objectStub->ListDocuments( &context, request, &objectListReply );

        if ( status.ok() )
        {
            CAFFA_TRACE( "Got list of documents" );

            caffa::JsonSerializer serializer( caffa::rpc::GrpcClientObjectFactory::instance() );
            serializer.setSerializeDataValues( false );

            for ( auto documentId : objectListReply.document_id() )
            {
                grpc::ClientContext docContext;

                caffa::rpc::DocumentRequest request;
                request.set_document_id( documentId );
                auto session = std::make_unique<caffa::rpc::SessionMessage>();
                session->set_uuid( m_sessionUuid );
                request.set_allocated_session( session.release() );
                caffa::rpc::RpcObject objectReply;
                CAFFA_TRACE( "Calling GetDocument()" );
                auto status = m_objectStub->GetDocument( &docContext, request, &objectReply );
                std::unique_ptr<caffa::ObjectHandle> document =
                    caffa::rpc::ObjectService::createCafObjectFromRpc( &objectReply, serializer );
                documents.push_back( std::move( document ) );
            }
        }
        else
        {
            CAFFA_ERROR( "Failed to get document with server error message: " + status.error_message() );
            throw Exception( status );
        }
        return documents;
    }

    std::unique_ptr<caffa::ObjectHandle> getChildObject( const caffa::ObjectHandle* objectHandle,
                                                         const std::string&         fieldName ) const
    {
        std::unique_ptr<caffa::ObjectHandle> childObject;

        CAFFA_TRACE( "Get Child Object from field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );

        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        GenericScalar reply;
        grpc::Status  status = m_fieldStub->GetValue( &context, field, &reply );
        if ( status.ok() )
        {
            childObject = caffa::JsonSerializer( caffa::rpc::GrpcClientObjectFactory::instance() )
                              .setSerializeDataValues( false )
                              .createObjectFromString( reply.value() );
        }
        else
        {
            CAFFA_ERROR( "Failed to get object with server error message: " + status.error_message() );
            throw Exception( status );
        }

        return childObject;
    }

    std::vector<std::unique_ptr<ObjectHandle>> getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                const std::string&         getter ) const
    {
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<std::unique_ptr<ObjectHandle>> childObjects;

        caffa::JsonSerializer serializer( caffa::rpc::GrpcClientObjectFactory::instance() );
        serializer.setSerializeDataValues( false );

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_objects() ); // TODO: throw
            RpcObjectList rpcObjects = reply.objects();

            for ( auto rpcObject : rpcObjects.objects() )
            {
                auto object = caffa::rpc::ObjectService::createCafObjectFromRpc( &rpcObject, serializer );
                childObjects.push_back( std::move( object ) );
            }
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return childObjects;
    }

    void setChildObject( const caffa::ObjectHandle* objectHandle,
                         const std::string&         fieldName,
                         const caffa::ObjectHandle* childObject )
    {
        CAFFA_TRACE( "Set Child Object in field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        auto                rpcChildObject = std::make_unique<RpcObject>();
        ObjectService::copyProjectObjectFromCafToRpc( childObject, rpcChildObject.get() );

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( fieldName );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        SetterRequest setterRequest;
        setterRequest.set_allocated_field( field.release() );
        setterRequest.set_value( rpcChildObject->json() );

        NullMessage  reply;
        grpc::Status status = m_fieldStub->SetValue( &context, setterRequest, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to set object" );
            throw Exception( status );
        }
    }

    void insertChildObject( const caffa::ObjectHandle* objectHandle,
                            const std::string&         fieldName,
                            size_t                     index,
                            const caffa::ObjectHandle* childObject )
    {
        CAFFA_TRACE( "Set Child Object in field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        auto                rpcChildObject = std::make_unique<RpcObject>();
        ObjectService::copyResultOrParameterObjectFromCafToRpc( childObject, rpcChildObject.get() );
        auto field = std::make_unique<FieldRequest>();

        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( fieldName );
        field->set_index( index );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        SetterRequest setterRequest;
        setterRequest.set_allocated_field( field.release() );
        setterRequest.set_value( rpcChildObject->json() );

        NullMessage  reply;
        grpc::Status status = m_fieldStub->InsertChildObject( &context, setterRequest, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to insert object" );
            throw Exception( status );
        }
    }

    void clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName )
    {
        CAFFA_TRACE( "Clear Child Objects from field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        NullMessage  reply;
        grpc::Status status = m_fieldStub->ClearChildObjects( &context, field, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to clear child objects" );
            throw Exception( status );
        }
    }

    void removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index )
    {
        CAFFA_TRACE( "Remove Child Object " << index << " from field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );
        field.set_index( index );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        NullMessage  reply;
        grpc::Status status = m_fieldStub->RemoveChildObject( &context, field, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to remove child object" );
            throw Exception( status );
        }
    }

    std::unique_ptr<caffa::ObjectMethodResult> execute( const caffa::ObjectMethod* method ) const
    {
        auto self   = std::make_unique<RpcObject>();
        auto params = std::make_unique<RpcObject>();
        ObjectService::copyProjectSelfReferenceFromCafToRpc( method->self<caffa::ObjectHandle>(), self.get() );
        ObjectService::copyResultOrParameterObjectFromCafToRpc( method, params.get() );

        grpc::ClientContext context;
        MethodRequest       request;
        request.set_allocated_self_object( self.release() );
        request.set_method( method->classKeyword() );
        request.set_allocated_params( params.release() );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        request.set_allocated_session( session.release() );

        std::unique_ptr<caffa::ObjectHandle> returnValue;

        caffa::rpc::RpcObject objectReply;
        auto                  status = m_objectStub->ExecuteMethod( &context, request, &objectReply );
        if ( status.ok() )
        {
            returnValue = caffa::rpc::ObjectService::createCafObjectFromRpc( &objectReply, caffa::JsonSerializer() );
        }
        else
        {
            CAFFA_ERROR( "Failed to execute object method " << method->classKeyword()
                                                            << " error: " << status.error_message() );
            throw Exception( status );
        }

        auto objectMethodResult = caffa::dynamic_unique_cast<caffa::ObjectMethodResult>( std::move( returnValue ) );
        return objectMethodResult;
    }

    bool stopServer()
    {
        destroySession();

        grpc::ClientContext context;
        NullMessage         nullarg, nullreply;
        CAFFA_DEBUG( "Telling server to quit" );
        auto status = m_appInfoStub->Quit( &context, nullarg, &nullreply );

        CAFFA_DEBUG( " Got the following response: " << status.ok() );
        return status.ok();
    }

    bool ping() const
    {
        grpc::ClientContext context;
        NullMessage         nullarg, nullreply;
        auto                status = m_appInfoStub->Ping( &context, nullarg, &nullreply );
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    void resetToDefaultData() const
    {
        grpc::ClientContext context;
        NullMessage         nullarg, nullreply;
        auto                status = m_appInfoStub->ResetToDefaultData( &context, nullarg, &nullreply );
        if ( !status.ok() )
        {
            throw Exception( status );
        }
    }

    std::list<std::unique_ptr<caffa::ObjectHandle>> objectMethods( caffa::ObjectHandle* objectHandle ) const
    {
        grpc::ClientContext context;
        auto                request = std::make_unique<ListMethodsRequest>();
        auto                self    = std::make_unique<RpcObject>();
        ObjectService::copyProjectObjectFromCafToRpc( objectHandle, self.get() );

        request->set_allocated_self_object( self.release() );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        request->set_allocated_session( session.release() );

        RpcObjectList reply;
        auto          status = m_objectStub->ListMethods( &context, *request, &reply );

        std::list<std::unique_ptr<caffa::ObjectHandle>> methods;
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to get object methods with error: " + status.error_message() );
            throw Exception( status );
        }

        for ( auto RpcObject : reply.objects() )
        {
            std::unique_ptr<caffa::ObjectHandle> caffaObject =
                ObjectService::createCafObjectMethodFromRpc( objectHandle,
                                                             &RpcObject,
                                                             caffa::ObjectMethodFactory::instance(),
                                                             caffa::rpc::GrpcClientObjectFactory::instance() );
            methods.push_back( std::move( caffaObject ) );
        }
        return methods;
    }

    void setJson( const caffa::ObjectHandle* objectHandle,
                  const std::string&         fieldName,
                  const nlohmann::json&      jsonValue,
                  uint32_t                   addressOffset )
    {
        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( fieldName );
        field->set_index( addressOffset );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        SetterRequest setterRequest;
        setterRequest.set_allocated_field( field.release() );
        setterRequest.set_value( jsonValue.dump() );

        NullMessage reply;
        auto        status = m_fieldStub->SetValue( &context, setterRequest, &reply );

        if ( !status.ok() )
        {
            throw Exception( status );
        }
    }

    nlohmann::json getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, uint32_t addressOffset ) const
    {
        CAFFA_TRACE( "Get JSON value for field " << fieldName << " on class " << objectHandle->classKeywordDynamic()
                                                 << " and UUID " << objectHandle->uuid() );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );
        field.set_index( addressOffset );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        GenericScalar reply;
        grpc::Status  status = m_fieldStub->GetValue( &context, field, &reply );
        if ( !status.ok() )
        {
            Exception e( status );
            CAFFA_ERROR( e.what() );
            throw e;
        }

        nlohmann::json jsonValue;

        CAFFA_TRACE( "Got scalar reply: " << reply.value() );
        jsonValue = nlohmann::json::parse( reply.value() );
        CAFFA_TRACE( "Got json value: " << jsonValue )
        return jsonValue;
    }

    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<int>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( setter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        auto setterRequest = std::make_unique<ArrayRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_field( field.release() );

        SetterArrayReply                                  reply;
        std::unique_ptr<grpc::ClientWriter<GenericArray>> writer( m_fieldStub->SetArrayValue( &context, &reply ) );
        GenericArray                                      header;
        header.set_allocated_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            GenericArray chunk;
            chunk.mutable_ints()->mutable_data()->Reserve( currentChunkSize );
            for ( size_t n = 0; n < currentChunkSize; ++n )
            {
                chunk.mutable_ints()->add_data( values[i + n] );
            }
            if ( !writer->Write( chunk ) ) return false;

            i += currentChunkSize;
        }
        if ( !writer->WritesDone() ) return false;

        grpc::Status status = writer->Finish();
        if ( !status.ok() && status.error_code() != grpc::OUT_OF_RANGE )
        {
            throw Exception( status );
        }
        return status.ok();
    }
    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<uint64_t>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( setter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        auto setterRequest = std::make_unique<ArrayRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_field( field.release() );

        SetterArrayReply                                  reply;
        std::unique_ptr<grpc::ClientWriter<GenericArray>> writer( m_fieldStub->SetArrayValue( &context, &reply ) );
        GenericArray                                      header;
        header.set_allocated_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            GenericArray chunk;
            chunk.mutable_uint64s()->mutable_data()->Reserve( currentChunkSize );
            for ( size_t n = 0; n < currentChunkSize; ++n )
            {
                chunk.mutable_uint64s()->add_data( values[i + n] );
            }
            if ( !writer->Write( chunk ) ) return false;

            i += currentChunkSize;
        }
        if ( !writer->WritesDone() ) return false;

        grpc::Status status = writer->Finish();
        if ( !status.ok() && status.error_code() != grpc::OUT_OF_RANGE )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<double>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        CAFFA_TRACE( "Sending " << values.size() << " double values from client" );
        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( setter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        auto setterRequest = std::make_unique<ArrayRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_field( field.release() );

        SetterArrayReply                                  reply;
        std::unique_ptr<grpc::ClientWriter<GenericArray>> writer( m_fieldStub->SetArrayValue( &context, &reply ) );
        GenericArray                                      header;
        header.set_allocated_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            GenericArray chunk;
            chunk.mutable_doubles()->mutable_data()->Reserve( currentChunkSize );
            for ( size_t n = 0; n < currentChunkSize; ++n )
            {
                chunk.mutable_doubles()->add_data( values[i + n] );
            }
            if ( !writer->Write( chunk ) ) return false;

            i += currentChunkSize;
        }
        if ( !writer->WritesDone() ) return false;

        grpc::Status status = writer->Finish();
        if ( !status.ok() && status.error_code() != grpc::OUT_OF_RANGE )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<float>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( setter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        auto setterRequest = std::make_unique<ArrayRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_field( field.release() );

        SetterArrayReply                                  reply;
        std::unique_ptr<grpc::ClientWriter<GenericArray>> writer( m_fieldStub->SetArrayValue( &context, &reply ) );
        GenericArray                                      header;
        header.set_allocated_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            GenericArray chunk;
            chunk.mutable_floats()->mutable_data()->Reserve( currentChunkSize );
            for ( size_t n = 0; n < currentChunkSize; ++n )
            {
                chunk.mutable_floats()->add_data( values[i + n] );
            }
            if ( !writer->Write( chunk ) ) return false;

            i += currentChunkSize;
        }
        if ( !writer->WritesDone() ) return false;

        grpc::Status status = writer->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<std::string>& values )
    {
        grpc::ClientContext context;

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( objectHandle->classKeywordDynamic() );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( setter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        auto setterRequest = std::make_unique<ArrayRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_field( field.release() );

        SetterArrayReply                                  reply;
        std::unique_ptr<grpc::ClientWriter<GenericArray>> writer( m_fieldStub->SetArrayValue( &context, &reply ) );
        GenericArray                                      header;
        header.set_allocated_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); ++i )
        {
            GenericArray chunk;
            chunk.mutable_strings()->add_data( values[i] );
            if ( !writer->Write( chunk ) ) return false;
        }
        if ( !writer->WritesDone() ) return false;

        grpc::Status status = writer->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    std::vector<int> getInts( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<int> values;

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_ints() ); // TODO: throw
            auto ints = reply.ints();
            values.insert( values.end(), ints.data().begin(), ints.data().end() );
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return values;
    }
    std::vector<uint64_t> getUInt64s( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<uint64_t> values;

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_uint64s() ); // TODO: throw
            auto ints = reply.uint64s();
            values.insert( values.end(), ints.data().begin(), ints.data().end() );
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return values;
    }

    std::vector<double> getDoubles( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<double> values;

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_doubles() ); // TODO: throw
            auto doubles = reply.doubles();
            values.insert( values.end(), doubles.data().begin(), doubles.data().end() );
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return values;
    }

    std::vector<float> getFloats( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<float> values;

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_floats() ); // TODO: throw
            auto floats = reply.floats();
            values.insert( values.end(), floats.data().begin(), floats.data().end() );
        }

        grpc::Status status = reader->Finish();
        if ( !status.ok() ) CAFFA_ERROR( "GRPC: " << status.error_code() << ", " << status.error_message() );
        CAFFA_ASSERT( status.ok() );
        return values;
    }

    std::vector<std::string> getStrings( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( objectHandle->classKeywordDynamic() );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<std::string> values;

        std::unique_ptr<grpc::ClientReader<GenericArray>> reader( m_fieldStub->GetArrayValue( &context, field ) );
        GenericArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_strings() ); // TODO: throw
            auto strings = reply.strings();
            values.insert( values.end(), strings.data().begin(), strings.data().end() );
        }
        grpc::Status status = reader->Finish();
        CAFFA_ASSERT( status.ok() );
        return values;
    }

private:
    std::shared_ptr<grpc::Channel> m_channel;

    std::unique_ptr<App::Stub>          m_appInfoStub;
    std::unique_ptr<ObjectAccess::Stub> m_objectStub;
    std::unique_ptr<FieldAccess::Stub>  m_fieldStub;

    std::string m_sessionUuid;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client::Client( const std::string& hostname,
                int                port /*= 50000 */,
                const std::string& clientCertFile,
                const std::string& clientKeyFile,
                const std::string& caCertFile )
    : m_clientImpl( std::make_unique<ClientImpl>( hostname, port, clientCertFile, clientKeyFile, caCertFile ) )
{
    caffa::rpc::GrpcClientObjectFactory::instance()->setGrpcClient( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client::~Client()
{
}

//--------------------------------------------------------------------------------------------------
/// Retrieve Application information
//--------------------------------------------------------------------------------------------------
caffa::AppInfo Client::appInfo() const
{
    return m_clientImpl->appInfo();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a top level document (project)
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> Client::document( const std::string& documentId ) const
{
    return m_clientImpl->document( documentId );
}

//--------------------------------------------------------------------------------------------------
/// Retrieve all top level documents
//--------------------------------------------------------------------------------------------------
std::vector<std::unique_ptr<caffa::ObjectHandle>> Client::documents() const
{
    return m_clientImpl->documents();
}

//--------------------------------------------------------------------------------------------------
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectMethodResult> Client::execute( caffa::not_null<const caffa::ObjectMethod*> method ) const
{
    return m_clientImpl->execute( method );
}

//--------------------------------------------------------------------------------------------------
/// Tell the server to stop operation. Returns a simple boolean status where true is ok.
//--------------------------------------------------------------------------------------------------
bool Client::stopServer()
{
    return m_clientImpl->stopServer();
}

//--------------------------------------------------------------------------------------------------
// Tell the server to stay alive
//--------------------------------------------------------------------------------------------------
void Client::sendKeepAlive()
{
    m_clientImpl->sendKeepAlive();
}

//--------------------------------------------------------------------------------------------------
// Tell the server to destroy the session
//--------------------------------------------------------------------------------------------------
void Client::destroySession()
{
    m_clientImpl->destroySession();
}

//--------------------------------------------------------------------------------------------------
// Get the current session ID
//--------------------------------------------------------------------------------------------------
const std::string& Client::sessionUuid() const
{
    return m_clientImpl->sessionUuid();
}

//--------------------------------------------------------------------------------------------------
/// Send a ping to the server
//--------------------------------------------------------------------------------------------------
bool Client::ping() const
{
    return m_clientImpl->ping();
}

//--------------------------------------------------------------------------------------------------
/// Ask server to reset to default data
//--------------------------------------------------------------------------------------------------
void Client::resetToDefaultData() const
{
    return m_clientImpl->resetToDefaultData();
}

//--------------------------------------------------------------------------------------------------
/// Get a list of all object methods available for object
//--------------------------------------------------------------------------------------------------
std::list<std::unique_ptr<caffa::ObjectHandle>> Client::objectMethods( caffa::ObjectHandle* objectHandle ) const
{
    return m_clientImpl->objectMethods( objectHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::setJson( const caffa::ObjectHandle* objectHandle,
                      const std::string&         fieldName,
                      const nlohmann::json&      value,
                      uint32_t                   addressOffset )
{
    m_clientImpl->setJson( objectHandle, fieldName, value, addressOffset );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json
    Client::getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, uint32_t addressOffset ) const
{
    CAFFA_ASSERT( m_clientImpl && "Client not properly initialized" );
    return m_clientImpl->getJson( objectHandle, fieldName, addressOffset );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<int>&    value,
                              uint32_t                   addressOffset )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle*   objectHandle,
                              const std::string&           fieldName,
                              const std::vector<uint64_t>& value,
                              uint32_t                     addressOffset )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<double>& value,
                              uint32_t                   addressOffset )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<float>&  value,
                              uint32_t                   addressOffset )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle*      objectHandle,
                              const std::string&              fieldName,
                              const std::vector<std::string>& value,
                              uint32_t                        addressOffset )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<int> Client::get<std::vector<int>>( const caffa::ObjectHandle* objectHandle,
                                                const std::string&         fieldName,
                                                uint32_t                   addressOffset ) const
{
    return m_clientImpl->getInts( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<uint64_t> Client::get<std::vector<uint64_t>>( const caffa::ObjectHandle* objectHandle,
                                                          const std::string&         fieldName,
                                                          uint32_t                   addressOffset ) const
{
    return m_clientImpl->getUInt64s( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<double> Client::get<std::vector<double>>( const caffa::ObjectHandle* objectHandle,
                                                      const std::string&         fieldName,
                                                      uint32_t                   addressOffset ) const
{
    return m_clientImpl->getDoubles( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<float> Client::get<std::vector<float>>( const caffa::ObjectHandle* objectHandle,
                                                    const std::string&         fieldName,
                                                    uint32_t                   addressOffset ) const
{
    return m_clientImpl->getFloats( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<std::string> Client::get<std::vector<std::string>>( const caffa::ObjectHandle* objectHandle,
                                                                const std::string&         fieldName,
                                                                uint32_t                   addressOffset ) const
{
    return m_clientImpl->getStrings( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> Client::getChildObject( const caffa::ObjectHandle* objectHandle,
                                                             const std::string&         fieldName ) const
{
    return m_clientImpl->getChildObject( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::unique_ptr<caffa::ObjectHandle>> Client::getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                           const std::string&         fieldName ) const
{
    return m_clientImpl->getChildObjects( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::setChildObject( const caffa::ObjectHandle* objectHandle,
                             const std::string&         fieldName,
                             const caffa::ObjectHandle* childObject )
{
    return m_clientImpl->setChildObject( objectHandle, fieldName, childObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index )
{
    m_clientImpl->removeChildObject( objectHandle, fieldName, index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName )
{
    m_clientImpl->clearChildObjects( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::insertChildObject( const caffa::ObjectHandle* objectHandle,
                                const std::string&         fieldName,
                                size_t                     index,
                                const caffa::ObjectHandle* childObject )
{
    m_clientImpl->insertChildObject( objectHandle, fieldName, index, childObject );
}

} // namespace caffa::rpc
