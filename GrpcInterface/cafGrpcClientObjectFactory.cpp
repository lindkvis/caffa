#include "cafGrpcClientObjectFactory.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDefaultObjectFactory.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcChildArrayFieldAccessor.h"
#include "cafGrpcChildFieldAccessor.h"
#include "cafGrpcClient.h"
#include "cafGrpcDataFieldAccessor.h"
#include "cafGrpcException.h"

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

    CAFFA_TRACE( "Creating object of type " << classNameKeyword );

    auto objectHandle = caffa::DefaultObjectFactory::instance()->create( classNameKeyword );

    CAFFA_ASSERT( objectHandle );

    for ( auto field : objectHandle->fields() )
    {
        if ( field->keyword() != "uuid" && field->capability<FieldScriptingCapability>() != nullptr )
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
///
//--------------------------------------------------------------------------------------------------
void GrpcClientObjectFactory::applyAccessorToField( caffa::ObjectHandle* fieldOwner, caffa::FieldHandle* fieldHandle )
{
    CAFFA_ASSERT( m_grpcClient );
    if ( !m_grpcClient ) throw( Exception( grpc::Status( grpc::ABORTED, "No Client set in Grpc Client factory" ) ) );

    if ( auto childField = dynamic_cast<caffa::ChildFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildFieldAccessor>( m_grpcClient, childField ) );
    }
    else if ( auto childField = dynamic_cast<caffa::ChildArrayFieldHandle*>( fieldHandle ); childField )
    {
        childField->setAccessor( std::make_unique<GrpcChildArrayFieldAccessor>( m_grpcClient, childField ) );
    }
    else if ( auto dataField = dynamic_cast<caffa::DataField*>( fieldHandle ); dataField )
    {
        CAFFA_TRACE( "Looking for an accessor creator for data type: " << fieldHandle->dataType() );
        AccessorCreatorBase* accessorCreator = nullptr;
        for ( auto& [dataType, storedAccessorCreator] : m_accessorCreatorMap )
        {
            CAFFA_TRACE( "Found one for " << dataType << " is that right?" );
            if ( dataType == fieldHandle->dataType() )
            {
                CAFFA_TRACE( "Yes!" );
                accessorCreator = storedAccessorCreator.get();
                break;
            }
        }
        if ( !accessorCreator )
        {
            throw( Exception( grpc::Status( grpc::ABORTED,
                                            std::string( "Data type " ) + fieldHandle->dataType() +
                                                " not implemented in GRPC client" ) ) );
        }
        dataField->setUntypedAccessor( accessorCreator->create( m_grpcClient, fieldOwner, fieldHandle->keyword() ) );
    }
    else
    {
        CAFFA_ASSERT( false && "Datatype not implemented" );
    }
}

void GrpcClientObjectFactory::registerAccessorCreator( const std::string&                   dataType,
                                                       std::unique_ptr<AccessorCreatorBase> creator )
{
    CAFFA_DEBUG( "Registering accessor for data type: " << dataType );
    m_accessorCreatorMap.insert( std::make_pair( dataType, std::move( creator ) ) );
}

void GrpcClientObjectFactory::registerAllBasicAccessorCreators()
{
    registerBasicAccessorCreators<double>();
    registerBasicAccessorCreators<float>();
    registerBasicAccessorCreators<int>();
    registerBasicAccessorCreators<int64_t>();
    registerBasicAccessorCreators<unsigned>();
    registerBasicAccessorCreators<uint64_t>();
    registerBasicAccessorCreators<bool>();
    registerBasicAccessorCreators<std::string>();
}

} // namespace caffa::rpc
