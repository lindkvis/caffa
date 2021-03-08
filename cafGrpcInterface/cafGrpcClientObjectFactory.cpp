#include "cafGrpcClientObjectFactory.h"

#include "cafDataValueField.h"
#include "cafDefaultObjectFactory.h"
#include "cafGrpcClient.h"
#include "cafGrpcDataFieldAccessor.h"
#include "cafGrpcException.h"
#include "cafGrpcObjectClientCapability.h"

#include <memory>

namespace caf::rpc
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* GrpcClientObjectFactory::doCreate( const std::string& classNameKeyword, uint64_t addressOnServer )
{
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    auto objectHandle = caf::DefaultObjectFactory::instance()->create( classNameKeyword );
    if ( addressOnServer != 0u )
    {
        objectHandle->addCapability( new ObjectClientCapability( addressOnServer ), true );
    }

    for ( auto field : objectHandle->fields() )
    {
        applyAccessorToField( objectHandle, field );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GrpcClientObjectFactory::setGrpcClient( Client* client )
{
    m_grpcClient = client;
}

//--------------------------------------------------------------------------------------------------
/// TODO: fix this hard coded mess of if statements
//--------------------------------------------------------------------------------------------------
void GrpcClientObjectFactory::applyAccessorToField( caf::ObjectHandle* fieldOwner, caf::FieldHandle* fieldHandle )
{
    if ( auto dataField = dynamic_cast<caf::Field<double>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<double>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caf::Field<int>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<int>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caf::Field<std::string>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::string>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    if ( auto dataField = dynamic_cast<caf::Field<std::vector<double>>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<std::vector<double>>>( m_grpcClient,
                                                                                      fieldOwner,
                                                                                      fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caf::Field<std::vector<int>>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::vector<int>>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caf::Field<std::vector<std::string>>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<std::vector<std::string>>>( m_grpcClient,
                                                                                           fieldOwner,
                                                                                           fieldHandle->keyword() );
        dataField->setFieldDataAccessor( std::move( accessor ) );
    }
    else
    {
        CAF_ASSERT( "Datatype not implemented" );
    }
}

} // namespace caf::rpc
