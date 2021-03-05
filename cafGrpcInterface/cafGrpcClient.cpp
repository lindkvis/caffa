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
