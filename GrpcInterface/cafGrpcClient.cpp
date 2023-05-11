// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2021- Kontur AS
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
#include "cafGrpcClient.h"

#include "App.grpc.pb.h"
#include "ObjectService.grpc.pb.h"

#include "cafDefaultObjectFactory.h"
#include "cafGrpcApplication.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcException.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcObjectService.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafSession.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

namespace caffa::rpc
{
class ClientImpl
{
public:
    ClientImpl( caffa::Session::Type sessionType,
                const std::string&   hostname,
                int                  port,
                const std::string&   clientCertFile,
                const std::string&   clientKeyFile,
                const std::string&   caCertFile )
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
        CAFFA_TRACE( "Created channel for " << hostname << ":" << port );
        m_appInfoStub = App::NewStub( m_channel );
        m_objectStub  = ObjectAccess::NewStub( m_channel );
        m_fieldStub   = FieldAccess::NewStub( m_channel );
        CAFFA_TRACE( "Created stubs" );
        if ( sessionType != caffa::Session::Type::INVALID )
        {
            createSession( sessionType );
        }
    }
    ~ClientImpl()
    {
        //
        CAFFA_DEBUG( "Destroying client" );
        destroySession();
        if ( m_keepAliveThread )
        {
            m_keepAliveThread->join();
        }
    }

    const std::string& sessionUuid() const
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );
        return m_sessionUuid;
    }

    void createSession( caffa::Session::Type sessionType )
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );

        caffa::rpc::SessionParameters params;
        caffa::rpc::SessionMessage    session;
        grpc::ClientContext           context;
        NullMessage                   nullarg;

        params.set_type( static_cast<caffa::rpc::SessionType>( sessionType ) );

        auto status = m_appInfoStub->CreateSession( &context, params, &session );
        if ( !status.ok() )
        {
            throw Exception( status );
        }

        m_sessionUuid = session.uuid();
        CAFFA_DEBUG( "Created session " << m_sessionUuid );
    }

    void sendKeepAlive()
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );
        CAFFA_DEBUG( "Keeping session alive " << m_sessionUuid );
        if ( m_sessionUuid.empty() )
        {
            throw std::runtime_error( "No session to keep alive" );
        }

        caffa::rpc::SessionMessage session;
        session.set_uuid( m_sessionUuid );
        grpc::ClientContext context;
        SessionMessage      reply;

        auto status = m_appInfoStub->KeepSessionAlive( &context, session, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( status.error_message() );
            throw Exception( status );
        }
    }

    void startKeepAliveThread()
    {
        m_keepAliveThread = std::make_unique<std::thread>(
            [this]()
            {
                while ( true )
                {
                    try
                    {
                        this->sendKeepAlive();
                        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                    }
                    catch ( ... )
                    {
                        break;
                    }
                }
            } );
    }

    caffa::Session::Type checkSession() const
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );
        if ( !m_sessionUuid.empty() )
        {
            CAFFA_DEBUG( "Checking session " << m_sessionUuid );
            caffa::rpc::SessionMessage session;
            session.set_uuid( m_sessionUuid );
            grpc::ClientContext context;
            SessionMessage      reply;

            auto status = m_appInfoStub->CheckSession( &context, session, &reply );
            if ( !status.ok() )
            {
                CAFFA_ERROR( status.error_message() );
                throw Exception( status );
            }
            CAFFA_DEBUG( "Session exists and is of type " << static_cast<int>( reply.type() ) );
            return static_cast<caffa::Session::Type>( reply.type() );
        }
        return caffa::Session::Type::INVALID;
    }

    void changeSession( caffa::Session::Type newType )
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );
        if ( m_sessionUuid.empty() ) throw std::runtime_error( "No session to change" );

        CAFFA_DEBUG( "Changing session " << m_sessionUuid << " to " << static_cast<int>( newType ) );

        caffa::rpc::SessionMessage session;
        session.set_uuid( m_sessionUuid );
        session.set_type( static_cast<caffa::rpc::SessionType>( newType ) );

        grpc::ClientContext context;
        SessionMessage      reply;

        auto status = m_appInfoStub->ChangeSession( &context, session, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( status.error_message() );
            throw Exception( status );
        }
    }

    void destroySession()
    {
        std::scoped_lock<std::mutex> lock( m_sessionMutex );
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
        std::scoped_lock<std::mutex> lock( m_sessionMutex );

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
        std::scoped_lock<std::mutex> lock( m_sessionMutex );

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

    std::unique_ptr<caffa::ObjectHandle> getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                      const std::string&         fieldName ) const
    {
        std::unique_ptr<caffa::ObjectHandle> childObject;

        CAFFA_TRACE( "Get Child Object from field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );

        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        GenericValue reply;
        grpc::Status status = m_fieldStub->GetValue( &context, field, &reply );
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

    std::unique_ptr<caffa::ObjectHandle> getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                   const std::string&         fieldName ) const
    {
        std::unique_ptr<caffa::ObjectHandle> childObject;

        CAFFA_TRACE( "Get Child Object from field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );
        field.set_copy_object_values( true );

        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        GenericValue reply;
        grpc::Status status = m_fieldStub->GetValue( &context, field, &reply );
        if ( status.ok() )
        {
            childObject = caffa::JsonSerializer( caffa::DefaultObjectFactory::instance() )
                              .setSerializeDataValues( true )
                              .createObjectFromString( reply.value() );
        }
        else
        {
            CAFFA_ERROR( "Failed to get object with server error message: " + status.error_message() );
            throw Exception( status );
        }

        return childObject;
    }

    void deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                  const std::string&         fieldName,
                                  const caffa::ObjectHandle* childObject )
    {
        CAFFA_INFO( "Copying Child Object back for field " << fieldName );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        auto                rpcChildObject = std::make_unique<RpcObject>();
        ObjectService::copyResultOrParameterObjectFromCafToRpc( childObject, rpcChildObject.get() );

        auto field = std::make_unique<FieldRequest>();
        field->set_class_keyword( std::string( objectHandle->classKeyword() ) );
        field->set_uuid( objectHandle->uuid() );
        field->set_keyword( fieldName );
        field->set_copy_object_values( true );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field->set_allocated_session( session.release() );

        SetterRequest setterRequest;
        setterRequest.set_allocated_field( field.release() );
        setterRequest.set_value( rpcChildObject->json() );

        CAFFA_INFO( "Performing SetValue" );
        NullMessage  reply;
        grpc::Status status = m_fieldStub->SetValue( &context, setterRequest, &reply );
        if ( !status.ok() )
        {
            CAFFA_ERROR( "Failed to set object" );
            throw Exception( status );
        }
    }

    std::vector<ObjectHandle::Ptr> getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                const std::string&         getter ) const
    {
        grpc::ClientContext context;

        FieldRequest field;
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( getter );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        std::vector<ObjectHandle::Ptr> childObjects;

        caffa::JsonSerializer serializer( caffa::rpc::GrpcClientObjectFactory::instance() );
        serializer.setSerializeDataValues( false );

        GenericValue reply;
        grpc::Status status = m_fieldStub->GetValue( &context, field, &reply );
        if ( status.ok() )
        {
            CAFFA_DEBUG( "Now serializing result to child object " << reply.value() );
            nlohmann::json jsonArray = nlohmann::json::parse( reply.value() );
            for ( auto arrayEntry : jsonArray )
            {
                auto childObject = caffa::JsonSerializer( caffa::rpc::GrpcClientObjectFactory::instance() )
                                       .setSerializeDataValues( false )
                                       .createObjectFromString( arrayEntry.dump() );
                childObjects.push_back( std::move( childObject ) );
            }
        }
        else
        {
            CAFFA_ERROR( "Failed to get object with server error message: " + status.error_message() );
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
        field->set_class_keyword( std::string( objectHandle->classKeyword() ) );
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

        field->set_class_keyword( std::string( objectHandle->classKeyword() ) );
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
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
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
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
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
        request.set_method( std::string( method->classKeyword() ) );
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
        std::scoped_lock<std::mutex> lock( m_sessionMutex );

        grpc::ClientContext context;
        SessionMessage      sessionRequest;
        sessionRequest.set_uuid( m_sessionUuid );
        NullMessage nullreply;
        CAFFA_DEBUG( "Telling server to quit" );

        auto status = m_appInfoStub->Quit( &context, sessionRequest, &nullreply );
        if ( status.ok() )
        {
            CAFFA_DEBUG( "Successfully quit" );
            m_sessionUuid = "";
            return status.ok();
        }
        else
        {
            CAFFA_ERROR( "Failed to quit because of: " + status.error_message() );
            throw Exception( status );
        }
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
        field->set_class_keyword( std::string( objectHandle->classKeyword() ) );
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
        CAFFA_TRACE( "Get JSON value for field " << fieldName << " on class " << objectHandle->classKeyword()
                                                 << " and UUID " << objectHandle->uuid() );
        CAFFA_ASSERT( m_fieldStub.get() && "Field Stub not initialized!" );
        grpc::ClientContext context;
        FieldRequest        field;
        field.set_class_keyword( std::string( objectHandle->classKeyword() ) );
        field.set_uuid( objectHandle->uuid() );
        field.set_keyword( fieldName );
        field.set_index( addressOffset );
        auto session = std::make_unique<caffa::rpc::SessionMessage>();
        session->set_uuid( m_sessionUuid );
        field.set_allocated_session( session.release() );

        GenericValue reply;
        grpc::Status status = m_fieldStub->GetValue( &context, field, &reply );
        if ( !status.ok() )
        {
            Exception e( status );
            CAFFA_ERROR( e.what() );
            throw e;
        }

        nlohmann::json jsonValue;

        CAFFA_TRACE( "Got scalar reply: " << reply.value() );
        jsonValue = nlohmann::json::parse( reply.value() );
        CAFFA_TRACE( "Got json value: " << jsonValue );
        return jsonValue;
    }

private:
    std::shared_ptr<grpc::Channel> m_channel;

    std::unique_ptr<App::Stub>          m_appInfoStub;
    std::unique_ptr<ObjectAccess::Stub> m_objectStub;
    std::unique_ptr<FieldAccess::Stub>  m_fieldStub;

    std::string                  m_sessionUuid;
    std::unique_ptr<std::thread> m_keepAliveThread;
    mutable std::mutex           m_sessionMutex;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client::Client( caffa::Session::Type sessionType,
                const std::string&   hostname,
                int                  port /*= 50000 */,
                const std::string&   clientCertFile,
                const std::string&   clientKeyFile,
                const std::string&   caCertFile )
    : m_clientImpl( std::make_unique<ClientImpl>( sessionType, hostname, port, clientCertFile, clientKeyFile, caCertFile ) )
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
// Start sending keep-alives in a thread until the session is destroyed.
//--------------------------------------------------------------------------------------------------
void Client::startKeepAliveThread()
{
    m_clientImpl->startKeepAliveThread();
}

//--------------------------------------------------------------------------------------------------
// Check the current session
//--------------------------------------------------------------------------------------------------
caffa::Session::Type Client::checkSession() const
{
    return m_clientImpl->checkSession();
}

//--------------------------------------------------------------------------------------------------
// Change the session type
//--------------------------------------------------------------------------------------------------
void Client::changeSession( caffa::Session::Type newType )
{
    m_clientImpl->changeSession( newType );
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
std::unique_ptr<caffa::ObjectHandle> Client::getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                          const std::string&         fieldName ) const
{
    return m_clientImpl->getShallowCopyOfChildObject( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> Client::getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                       const std::string&         fieldName ) const
{
    return m_clientImpl->getDeepCopyOfChildObject( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                      const std::string&         fieldName,
                                      const caffa::ObjectHandle* childObject )
{
    m_clientImpl->deepCopyChildObjectFrom( objectHandle, fieldName, childObject );
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
