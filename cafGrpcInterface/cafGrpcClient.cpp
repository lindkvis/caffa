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

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>
#include <memory>

namespace caf::rpc
{
class ClientImpl
{
public:
    ClientImpl( const std::string& hostname, int port )
    {
        // Created new server
        m_channel = grpc::CreateChannel( hostname + ":" + std::to_string( port ), grpc::InsecureChannelCredentials() );
        std::cout << "Created channel for " << hostname << ":" << port << std::endl;
        m_appInfoStub = App::NewStub( m_channel );
        m_objectStub  = ObjectAccess::NewStub( m_channel );
        m_fieldStub   = FieldAccess::NewStub( m_channel );
        std::cout << "Created stubs" << std::endl;
    }

    caf::AppInfo appInfo() const
    {
        caf::rpc::AppInfoReply reply;
        grpc::ClientContext    context;
        NullMessage            nullarg;
        auto                   status  = m_appInfoStub->GetAppInfo( &context, nullarg, &reply );
        caf::AppInfo           appInfo = { reply.name(),
                                 reply.major_version(),
                                 reply.minor_version(),
                                 reply.patch_version(),
                                 reply.type() };
        return appInfo;
    }

    std::unique_ptr<caf::ObjectHandle> document( const std::string& documentId ) const
    {
        std::unique_ptr<caf::ObjectHandle> pdmDocument;

        grpc::ClientContext       context;
        caf::rpc::DocumentRequest request;
        request.set_document_id( documentId );
        caf::rpc::Object objectReply;
        std::cout << "Calling GetDocument()" << std::endl;
        auto status = m_objectStub->GetDocument( &context, request, &objectReply );
        if ( status.ok() )
        {
            std::cout << "Got document" << std::endl;
            pdmDocument = caf::rpc::ObjectService::createCafObjectFromRpc( &objectReply,
                                                                           caf::rpc::GrpcClientObjectFactory::instance() );
        }
        else
        {
            std::cout << "Failed to get document" << std::endl;
        }
        return pdmDocument;
    }

    std::unique_ptr<caf::ObjectHandle> execute( const caf::ObjectMethod* method ) const
    {
        auto self   = std::make_unique<Object>();
        auto params = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( method->self<caf::ObjectHandle>(),
                                               self.get(),
                                               caf::DefaultObjectFactory::instance() );
        ObjectService::copyObjectFromCafToRpc( method, params.get(), caf::DefaultObjectFactory::instance() );

        grpc::ClientContext context;
        MethodRequest       request;
        request.set_allocated_self( self.release() );
        request.set_method( method->classKeyword() );
        request.set_allocated_params( params.release() );

        std::unique_ptr<caf::ObjectHandle> returnValue;

        caf::rpc::Object objectReply;
        auto             status = m_objectStub->ExecuteMethod( &context, request, &objectReply );
        if ( status.ok() )
        {
            returnValue =
                caf::rpc::ObjectService::createCafObjectFromRpc( &objectReply, caf::DefaultObjectFactory::instance() );
        }
        return returnValue;
    }

    bool stopServer() const
    {
        grpc::ClientContext context;
        NullMessage         nullarg, nullreply;
        auto                status = m_appInfoStub->Quit( &context, nullarg, &nullreply );
        return status.ok();
    }

    bool ping() const
    {
        grpc::ClientContext context;
        NullMessage         nullarg, nullreply;
        auto                status = m_appInfoStub->Ping( &context, nullarg, &nullreply );
        return status.ok();
    }

    void setJson( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& jsonValue )
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

    nlohmann::json getJson( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
    {
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
            jsonValue = nlohmann::json::parse( reply.scalar() );
        }
        grpc::Status status = reader->Finish();
        if ( !status.ok() )
        {
            throw Exception( status );
        }
        return jsonValue;
    }
    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<int>& values )
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
        return status.ok();
    }

    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<double>& values )
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
        return status.ok();
    }

    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<float>& values )
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
        return status.ok();
    }

    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<std::string>& values )
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
        return status.ok();
    }

    std::vector<int> getInts( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
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
            CAF_ASSERT( reply.has_ints() ); // TODO: throw
            auto ints = reply.ints();
            values.insert( values.end(), ints.data().begin(), ints.data().end() );
        }
        grpc::Status status = reader->Finish();
        CAF_ASSERT( status.ok() );
        return values;
    }

    std::vector<double> getDoubles( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
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
            CAF_ASSERT( reply.has_doubles() ); // TODO: throw
            auto doubles = reply.doubles();
            values.insert( values.end(), doubles.data().begin(), doubles.data().end() );
        }
        grpc::Status status = reader->Finish();
        CAF_ASSERT( status.ok() );
        return values;
    }

    std::vector<float> getFloats( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        FieldRequest field;
        field.set_method( getter );
        field.set_allocated_self( self.release() );

        std::vector<float> values;

        auto start_time = std::chrono::system_clock::now();

        std::unique_ptr<grpc::ClientReader<FloatArray>> reader( m_fieldStub->GetFloatValue( &context, field ) );
        FloatArray                                      reply;
        while ( reader->Read( &reply ) )
        {
            values.insert( values.end(), reply.data().begin(), reply.data().end() );
        }

        auto   end_time       = std::chrono::system_clock::now();
        auto   duration       = std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count();
        size_t numberOfFloats = values.size();
        size_t MB             = numberOfFloats * sizeof( float ) / ( 1024u * 1024u );
        std::cout << "Transferred " << numberOfFloats << " floats for a total of " << MB << " MB" << std::endl;
        std::cout << "Time spent: " << duration << "ms" << std::endl;

        grpc::Status status = reader->Finish();
        if ( !status.ok() ) std::cout << status.error_code() << ", " << status.error_message() << std::endl;
        CAF_ASSERT( status.ok() );
        return values;
    }

    std::vector<std::string> getStrings( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
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
            CAF_ASSERT( reply.has_strings() ); // TODO: throw
            auto strings = reply.strings();
            values.insert( values.end(), strings.data().begin(), strings.data().end() );
        }
        grpc::Status status = reader->Finish();
        CAF_ASSERT( status.ok() );
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
caf::AppInfo Client::appInfo() const
{
    return m_clientImpl->appInfo();
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a top level document (project)
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> Client::document( const std::string& documentId ) const
{
    return m_clientImpl->document( documentId );
}

//--------------------------------------------------------------------------------------------------
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> Client::execute( gsl::not_null<const caf::ObjectMethod*> method ) const
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
void Client::setJson( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value )
{
    m_clientImpl->setJson( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
nlohmann::json Client::getJson( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getJson( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caf::rpc::Client::set( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const std::vector<int>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caf::rpc::Client::set( const caf::ObjectHandle*   objectHandle,
                            const std::string&         fieldName,
                            const std::vector<double>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caf::rpc::Client::set( const caf::ObjectHandle*  objectHandle,
                            const std::string&        fieldName,
                            const std::vector<float>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
void caf::rpc::Client::set( const caf::ObjectHandle*        objectHandle,
                            const std::string&              fieldName,
                            const std::vector<std::string>& value )
{
    m_clientImpl->set( objectHandle, fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<int> Client::get<std::vector<int>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getInts( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<double>
    Client::get<std::vector<double>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getDoubles( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<float> Client::get<std::vector<float>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getFloats( objectHandle, fieldName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
std::vector<std::string>
    Client::get<std::vector<std::string>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    return m_clientImpl->getStrings( objectHandle, fieldName );
}

} // namespace caf::rpc
