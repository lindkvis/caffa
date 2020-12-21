#include "cafGrpcClientObjectFactory.h"

#include "cafDefaultObjectFactory.h"
#include "cafGrpcObjectClientCapability.h"

namespace caf::rpc
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* GrpcClientObjectFactory::doCreate( const std::string& classNameKeyword, uint64_t addressOnServer)
{
    auto objectHandle = caf::DefaultObjectFactory::instance()->create( classNameKeyword );
    if ( addressOnServer != 0u)
    {
        objectHandle->addCapability( new ObjectClientCapability( addressOnServer ), true );
    }
    return objectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> GrpcClientObjectFactory::classKeywords() const
{
    return caf::DefaultObjectFactory::instance()->classKeywords();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GrpcClientObjectFactory* GrpcClientObjectFactory::instance()
{
    static GrpcClientObjectFactory* fact = new GrpcClientObjectFactory;
    return fact;
}

} // End of namespace caf
