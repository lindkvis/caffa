#include "cafGrpcServerApplication.h"

#include "cafAssert.h"
#include "cafGrpcServer.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcServerApplication::GrpcServerApplication( int portNumber )
    : Application( AppCapability::GRPC_SERVER )

{
    m_server = std::make_unique<caf::rpc::Server>( portNumber );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcServerApplication* GrpcServerApplication::instance()
{
    Application* appInstance = Application::instance();
    return dynamic_cast<GrpcServerApplication*>( appInstance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcServerApplication::run()
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
void GrpcServerApplication::quit()
{
    m_server->quit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool GrpcServerApplication::running() const
{
    return m_server->running();
}
