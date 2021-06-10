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
#include "cafGrpcObjectClientCapability.h"
#include "cafGrpcObjectService.h"
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
    ClientImpl( const std::string& hostname, int port )
    {
        // Created new server
        m_channel = grpc::CreateChannel( hostname + ":" + std::to_string( port ), grpc::InsecureChannelCredentials() );
        CAFFA_DEBUG( "Created channel for " << hostname << ":" << port );
        m_appInfoStub = App::NewStub( m_channel );
        m_objectStub  = ObjectAccess::NewStub( m_channel );
        m_fieldStub   = FieldAccess::NewStub( m_channel );
        CAFFA_TRACE( "Created stubs" );
    }
    ~ClientImpl() { CAFFA_DEBUG( "Destroying client" ); }

    caffa::AppInfo appInfo() const
    {
        CAFFA_TRACE( "Trying to get app info" );
        caffa::rpc::AppInfoReply reply;
        grpc::ClientContext      context;
        NullMessage              nullarg;
        auto                     status  = m_appInfoStub->GetAppInfo( &context, nullarg, &reply );
        caffa::AppInfo           appInfo = { reply.name(),
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
        caffa::rpc::Object objectReply;
        CAFFA_TRACE( "Calling GetDocument()" );
        auto status = m_objectStub->GetDocument( &context, request, &objectReply );
        if ( status.ok() )
        {
            CAFFA_TRACE( "Got document" );
            document = caffa::rpc::ObjectService::createCafObjectFromRpc( &objectReply,
                                                                          caffa::rpc::GrpcClientObjectFactory::instance() );
            CAFFA_TRACE( "Document completed" );
        }
        else
        {
            CAFFA_ERROR( "Failed to get document" );
        }
        return document;
    }

    std::unique_ptr<caffa::ObjectHandle> execute( const caffa::ObjectMethod* method ) const
    {
        auto self   = std::make_unique<Object>();
        auto params = std::make_unique<Object>();
        CAFFA_TRACE( "Copying self" );
        ObjectService::copyObjectFromCafToRpc( method->self<caffa::ObjectHandle>(), self.get(), false, false );
        CAFFA_TRACE( "Copying parameters" );
        ObjectService::copyObjectFromCafToRpc( method, params.get(), true, true );

        grpc::ClientContext context;
        MethodRequest       request;
        request.set_allocated_self( self.release() );
        request.set_method( method->classKeyword() );
        request.set_allocated_params( params.release() );

        std::unique_ptr<caffa::ObjectHandle> returnValue;

        caffa::rpc::Object objectReply;
        auto               status = m_objectStub->ExecuteMethod( &context, request, &objectReply );
        if ( status.ok() )
        {
            returnValue = caffa::rpc::ObjectService::createCafObjectFromRpc( &objectReply,
                                                                             caffa::DefaultObjectFactory::instance() );
        }
        return returnValue;
    }

    bool stopServer() const
    {
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
        return status.ok();
    }

    void setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& jsonValue )
    {
        size_t chunkSize = 1;

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( fieldName );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( 1u );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );

        if ( writer->Write( header ) )
        {
            SetterChunk value;
            if ( jsonValue.is_string() )
                value.set_scalar( jsonValue.get<std::string>() );
            else
                value.set_scalar( jsonValue.dump() );
            writer->Write( value );
        }
        writer->WritesDone();
        auto status = writer->Finish();

        if ( !status.ok() )
        {
            throw Exception( status );
        }
    }

    nlohmann::json getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
    {
        CAFFA_TRACE( "Get JSON value for field " << fieldName );
        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( fieldName );
        field.set_allocated_self( self.release() );

        std::vector<int> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;

        nlohmann::json jsonValue;

        if ( reader->Read( &reply ) )
        {
            CAFFA_TRACE( "Got scalar reply: " << reply.scalar() );
            jsonValue = nlohmann::json::parse( reply.scalar() );
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        CAFFA_TRACE( "Got json value: " << jsonValue )
        return jsonValue;
    }
    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<int>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( setter );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            SetterChunk chunk;
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
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return status.ok();
    }
    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<uint64_t>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( setter );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            SetterChunk chunk;
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
        if ( !status.ok() )
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( setter );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            SetterChunk chunk;
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
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return status.ok();
    }

    bool set( const caffa::ObjectHandle* objectHandle, const std::string& setter, const std::vector<float>& values )
    {
        auto chunkSize = Application::instance()->packageByteSize();

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( setter );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); )
        {
            auto currentChunkSize = std::min( chunkSize, values.size() - i );

            SetterChunk chunk;
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto field = std::make_unique<FieldRequest>();
        field->set_method( setter );
        field->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( field.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_fieldStub->SetValue( &context, &reply ) );
        SetterChunk                                      header;
        header.set_allocated_set_request( setterRequest.release() );
        if ( !writer->Write( header ) ) return false;

        for ( size_t i = 0; i < values.size(); ++i )
        {
            SetterChunk chunk;
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<int> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<uint64_t> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<double> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;
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
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<float> values;

        auto start_time = std::chrono::system_clock::now();

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;
        while ( reader->Read( &reply ) )
        {
            CAFFA_ASSERT( reply.has_floats() ); // TODO: throw
            auto floats = reply.floats();
            values.insert( values.end(), floats.data().begin(), floats.data().end() );
        }

        auto   end_time       = std::chrono::system_clock::now();
        auto   duration       = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t numberOfFloats = values.size();
        size_t MB             = numberOfFloats * sizeof( float ) / ( 1024u * 1024u );

        grpc::Status status = reader->Finish();
        if ( !status.ok() ) CAFFA_ERROR( "GRPC: " << status.error_code() << ", " << status.error_message() );
        CAFFA_ASSERT( status.ok() );
        return values;
    }

    std::vector<std::string> getStrings( const caffa::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<std::string> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_fieldStub->GetValue( &context, field ) );
        GetterReply                                      reply;
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
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client::Client( const std::string& hostname, int port /*= 55555 */ )
    : m_clientImpl( std::make_unique<ClientImpl>( hostname, port ) )
{
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
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caffa::ObjectHandle> Client::execute( gsl::not_null<const caffa::ObjectMethod*> method ) const
{
    return m_clientImpl->execute( method );
}

//--------------------------------------------------------------------------------------------------
/// Tell the server to stop operation. Returns a simple boolean status where true is ok.
//--------------------------------------------------------------------------------------------------
bool Client::stopServer() const
{
    return m_clientImpl->stopServer();
}

bool Client::ping() const
{
    return m_clientImpl->ping();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Client::setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value )
{
    m_clientImpl->setJson( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json Client::getJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getJson( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<int>&    value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle*   objectHandle,
                              const std::string&           fieldName,
                              const std::vector<uint64_t>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<double>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const std::vector<float>&  value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caffa::rpc::Client::set( const caffa::ObjectHandle*      objectHandle,
                              const std::string&              fieldName,
                              const std::vector<std::string>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<int> Client::get<std::vector<int>>( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getInts( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<uint64_t>
    Client::get<std::vector<uint64_t>>( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getUInt64s( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<double>
    Client::get<std::vector<double>>( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getDoubles( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<float>
    Client::get<std::vector<float>>( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getFloats( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<std::string>
    Client::get<std::vector<std::string>>( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getStrings( objectHandle, fieldName );
}

} // namespace caffa::rpc
