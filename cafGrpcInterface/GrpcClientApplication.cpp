#include "cafGrpcClientApplication.h"

#include "cafAssert.h"
#include "cafGrpcClient.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClientApplication::GrpcClientApplication( int portNumber )
    : Application( { AppCapability::GRPC_CLIENT } )

{
    m_client = std::make_unique<caf::rpc::Client>( portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClientApplication* GrpcClientApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<GrpcClientApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcClientApplication::run()
{
    CAF_ASSERT( m_server );
    m_server->run();

    while ( !m_server->quitting() )
    {
        m_server->processAllRequests();
    }
    m_server->forceQuit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcClientApplication::quit()
{
    m_server->quit();
}
