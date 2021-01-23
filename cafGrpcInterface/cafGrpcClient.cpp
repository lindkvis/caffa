#include "cafGrpcClient.h"

#include "App.grpc.pb.h"
#include "Object.grpc.pb.h"

#include "cafDefaultObjectFactory.h"
#include "cafGrpcObjectService.h"
#include "cafGrpcObjectClientCapability.h"

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
        std::cout << "Attempting to contact server" << std::endl;
        auto         status  = m_appInfoStub->GetAppInfo( &context, nullarg, &reply );
        caf::AppInfo appInfo = { reply.name(),
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
        std::cout << "Attempting to get document" << std::endl;
        auto status = m_objectStub->GetDocument( &context, request, &objectReply );
        if ( status.ok() )
        {
            pdmDocument = caf::rpc::ObjectService::createCafObjectFromRpc( &objectReply );
        }
        return pdmDocument;
    }

    bool sync(caf::ObjectHandle* objectHandle)
    {
        grpc::ClientContext       context;
        caf::rpc::Object objectRequest, objectReply;
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
        std::cout << "Attempting to contact server";
        auto status = m_appInfoStub->Quit( &context, nullarg, &nullreply );
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
    : m_clientImpl( new ClientImpl( hostname, port ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Client::~Client()
{
    delete m_clientImpl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppInfo Client::appInfo() const
{
    return m_clientImpl->appInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Client::stopServer() const
{
    return m_clientImpl->stopServer();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> Client::document( const std::string& documentId ) const
{
    return m_clientImpl->document( documentId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Client::sync( caf::ObjectHandle* objectHandle )
{
    return m_clientImpl->sync( objectHandle );
}

} // namespace caf::rpc
