#include "cafGrpcServerApplication.h"

#include "cafAssert.h"
#include "cafGrpcAppService.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcObjectService.h"
#include "cafGrpcServer.h"
#include "cafGrpcServiceInterface.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcServerApplication::GrpcServerApplication( int portNumber )
    : Application( AppCapability::GRPC_SERVER )

{
    m_server = std::make_unique<caf::rpc::Server>( portNumber );

    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::AppService>( typeid( caf::rpc::AppService ).hash_code() );
    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::FieldService>(
        typeid( caf::rpc::FieldService ).hash_code() );
    caf::rpc::ServiceFactory::instance()->registerCreator<caf::rpc::ObjectService>(
        typeid( caf::rpc::ObjectService ).hash_code() );
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
