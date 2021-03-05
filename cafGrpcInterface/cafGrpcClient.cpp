#include "cafGrpcClient.h"

#include "App.grpc.pb.h"
#include "Object.grpc.pb.h"

#include "cafDefaultObjectFactory.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcObjectClientCapability.h"
#include "cafGrpcObjectService.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

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

        m_appInfoStub = App::NewStub( m_channel );
        m_objectStub  = ObjectAccess::NewStub( m_channel );
    }

    caf::AppInfo appInfo() const
    {
        caf::rpc::AppInfoReply reply;
        grpc::ClientContext    context;
        caf::rpc::Null         nullarg;
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
        auto             status = m_objectStub->GetDocument( &context, request, &objectReply );
        if ( status.ok() )
        {
            pdmDocument = caf::rpc::ObjectService::createCafObjectFromRpc( &objectReply,
                                                                           caf::rpc::GrpcClientObjectFactory::instance() );
        }
        return pdmDocument;
    }

    std::unique_ptr<caf::ObjectHandle> execute( const caf::ObjectMethod* method ) const
    {
        auto self   = std::make_unique<Object>();
        auto params = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( method->self<caf::ObjectHandle>(), self.get() );
        ObjectService::copyObjectFromCafToRpc( method, params.get() );

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
            returnValue = caf::rpc::ObjectService::createCafObjectFromRpc( &objectReply,
                                                                           caf::rpc::GrpcClientObjectFactory::instance() );
        }
        return returnValue;
    }

    bool sync( caf::ObjectHandle* objectHandle )
    {
        grpc::ClientContext context;
        caf::rpc::Object    objectRequest, objectReply;
        caf::rpc::ObjectService::copyObjectFromCafToRpc( objectHandle, &objectRequest );

        auto status = m_objectStub->Sync( &context, objectRequest, &objectReply );
        if ( status.ok() )
        {
            ObjectService::copyObjectFromRpcToCaf( &objectReply, objectHandle );
            return true;
        }
        return false;
    }

    bool stopServer() const
    {
        grpc::ClientContext context;
        caf::rpc::Null      nullarg, nullreply;
        auto                status = m_appInfoStub->Quit( &context, nullarg, &nullreply );
        return status.ok();
    }

    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<int>& values )
    {
        auto chunkSize = ServiceInterface::packageByteSize();

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto method = std::make_unique<MethodRequest>();
        method->set_method( setter );
        method->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( method.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_objectStub->ExecuteSetter( &context, &reply ) );
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
        auto chunkSize = ServiceInterface::packageByteSize();

        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto method = std::make_unique<MethodRequest>();
        method->set_method( setter );
        method->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( method.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_objectStub->ExecuteSetter( &context, &reply ) );
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

    bool set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<std::string>& values )
    {
        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );

        auto method = std::make_unique<MethodRequest>();
        method->set_method( setter );
        method->set_allocated_self( self.release() );

        auto setterRequest = std::make_unique<SetterRequest>();
        setterRequest->set_value_count( values.size() );
        setterRequest->set_allocated_request( method.release() );

        SetterReply                                      reply;
        std::unique_ptr<grpc::ClientWriter<SetterChunk>> writer( m_objectStub->ExecuteSetter( &context, &reply ) );
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
        MethodRequest method;
        method.set_method( getter );
        method.set_allocated_self( self.release() );

        std::vector<int> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_objectStub->ExecuteGetter( &context, method ) );
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
        MethodRequest method;
        method.set_method( getter );
        method.set_allocated_self( self.release() );

        std::vector<double> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_objectStub->ExecuteGetter( &context, method ) );
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

    std::vector<std::string> getStrings( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
    {
        grpc::ClientContext context;
        auto                self = std::make_unique<Object>();
        ObjectService::copyObjectFromCafToRpc( objectHandle, self.get(), false );
        MethodRequest method;
        method.set_method( getter );
        method.set_allocated_self( self.release() );

        std::vector<std::string> values;

        std::unique_ptr<grpc::ClientReader<GetterReply>> reader( m_objectStub->ExecuteGetter( &context, method ) );
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
///
//--------------------------------------------------------------------------------------------------
bool Client::set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<double>& values )
{
    return m_clientImpl->set( objectHandle, setter, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Client::set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<int>& values )
{
    return m_clientImpl->set( objectHandle, setter, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Client::set( const caf::ObjectHandle* objectHandle, const std::string& setter, const std::vector<std::string>& values )
{
    return m_clientImpl->set( objectHandle, setter, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> Client::getInts( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
{
    return m_clientImpl->getInts( objectHandle, getter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> Client::getDoubles( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
{
    return m_clientImpl->getDoubles( objectHandle, getter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> Client::getStrings( const caf::ObjectHandle* objectHandle, const std::string& getter ) const
{
    return m_clientImpl->getStrings( objectHandle, getter );
}

//--------------------------------------------------------------------------------------------------
/// Execute a general non-streaming method.
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> Client::execute( gsl::not_null<const caf::ObjectMethod*> method ) const
{
    return m_clientImpl->execute( method );
}

//--------------------------------------------------------------------------------------------------
/// Synchronise the object with the server. Returns true if the object was found and synchronised.
//--------------------------------------------------------------------------------------------------
bool Client::sync( gsl::not_null<caf::ObjectHandle*> objectHandle )
{
    return m_clientImpl->sync( objectHandle );
}

//--------------------------------------------------------------------------------------------------
/// Tell the server to stop operation. Returns a simple boolean status where true is ok.
//--------------------------------------------------------------------------------------------------
bool Client::stopServer() const
{
    return m_clientImpl->stopServer();
}

} // namespace caf::rpc
