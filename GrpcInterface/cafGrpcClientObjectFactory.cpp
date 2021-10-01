#include "cafGrpcClientObjectFactory.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDataValueField.h"
#include "cafDefaultObjectFactory.h"
#include "cafGrpcChildArrayFieldAccessor.h"
#include "cafGrpcChildFieldAccessor.h"
#include "cafGrpcClient.h"
#include "cafGrpcDataFieldAccessor.h"
#include "cafGrpcException.h"
#include "cafGrpcRegisterFieldAccessor.h"
#include "cafRegisterField.h"

#include <memory>

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> GrpcClientObjectFactory::doCreate( const std::string& classNameKeyword )
{
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    CAFFA_DEBUG( "Creating object of type " << classNameKeyword );

    auto objectHandle = caffa::DefaultObjectFactory::instance()->create( classNameKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" )
        {
            applyAccessorToField( objectHandle.get(), field );
        }
    }

    return objectHandle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> GrpcClientObjectFactory::classKeywords() const
{
    return caffa::DefaultObjectFactory::instance()->classKeywords();
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
void GrpcClientObjectFactory::applyAccessorToField( caffa::ObjectHandle* fieldOwner, caffa::FieldHandle* fieldHandle )
{
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    if ( auto dataField = dynamic_cast<caffa::Field<double>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<double>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<int>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<int>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<int64_t>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<int64_t>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }

    else if ( auto dataField = dynamic_cast<caffa::Field<bool>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<bool>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::string>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::string>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::string>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::string>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::vector<double>>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<std::vector<double>>>( m_grpcClient,
                                                                                      fieldOwner,
                                                                                      fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::vector<float>>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::vector<float>>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::vector<int>>*>( fieldHandle ); dataField )
    {
        auto accessor =
            std::make_unique<GrpcDataFieldAccessor<std::vector<int>>>( m_grpcClient, fieldOwner, fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::Field<std::vector<int64_t>>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<std::vector<int64_t>>>( m_grpcClient,
                                                                                       fieldOwner,
                                                                                       fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }

    else if ( auto dataField = dynamic_cast<caffa::Field<std::vector<std::string>>*>( fieldHandle ); dataField )
    {
        auto accessor = std::make_unique<GrpcDataFieldAccessor<std::vector<std::string>>>( m_grpcClient,
                                                                                           fieldOwner,
                                                                                           fieldHandle->keyword() );
        dataField->setAccessor( std::move( accessor ) );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildFieldAccessor>( m_grpcClient, childField ) );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildArrayFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildArrayFieldAccessor>( m_grpcClient, childField ) );
    }
    else if ( auto registerField = dynamic_cast<caffa::RegisterField*>( fieldHandle ); registerField )
    {
        registerField->setAccessor(
            std::make_unique<GrpcRegisterFieldAccessor>( m_grpcClient, fieldOwner, fieldHandle->keyword() ) );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

} // namespace caffa::rpc
