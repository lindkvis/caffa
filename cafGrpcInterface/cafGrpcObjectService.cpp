//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#include "cafGrpcObjectService.h"

#include "cafGrpcCallbacks.h"
#include "cafGrpcClientObjectFactory.h"
#include "cafGrpcServerApplication.h"

#include "cafAbstractFieldScriptingCapability.h"
#include "cafApplication.h"
#include "cafField.h"
#include "cafGrpcObjectClientCapability.h"
#include "cafObject.h"
#include "cafObjectMethod.h"
#include "cafObjectScriptingCapability.h"
#include "cafObjectScriptingCapabilityRegister.h"
#include "cafPdmDocument.h"
#include "cafPdmScriptIOMessages.h"
#include "cafProxyValueField.h"

#include <grpcpp/grpcpp.h>

#include <vector>

namespace caf::rpc
{
template <typename DataType>
struct DataHolder : public AbstractDataHolder
{
    DataHolder( const DataType& data )
        : data( data )
    {
    }

    size_t valueCount() const override { return data.size(); }
    size_t valueSizeOf() const override { return sizeof( typename DataType::value_type ); }

    void   reserveReplyStorage( GetterReply* reply ) const override;
    void   addValueToReply( size_t valueIndex, GetterReply* reply ) const override;
    size_t getValuesFromChunk( size_t startIndex, const SetterChunk* chunk ) override;
    void   applyValuesToProxyField( ProxyFieldHandle* proxyField ) override;

    DataType data;
};

template <>
void DataHolder<std::vector<int>>::reserveReplyStorage( GetterReply* reply ) const
{
    reply->mutable_ints()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<int>>::addValueToReply( size_t valueIndex, GetterReply* reply ) const
{
    reply->mutable_ints()->add_data( data[valueIndex] );
}
template <>
size_t DataHolder<std::vector<int>>::getValuesFromChunk( size_t startIndex, const SetterChunk* chunk )
{
    size_t chunkSize    = chunk->ints().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->ints().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<int>>::applyValuesToProxyField( ProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<ProxyValueField<std::vector<int>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

template <>
void DataHolder<std::vector<double>>::reserveReplyStorage( GetterReply* reply ) const
{
    reply->mutable_doubles()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<double>>::addValueToReply( size_t valueIndex, GetterReply* reply ) const
{
    reply->mutable_doubles()->add_data( data[valueIndex] );
}
template <>
size_t DataHolder<std::vector<double>>::getValuesFromChunk( size_t startIndex, const SetterChunk* chunk )
{
    size_t chunkSize    = chunk->doubles().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->doubles().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<double>>::applyValuesToProxyField( ProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<ProxyValueField<std::vector<double>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

template <>
void DataHolder<std::vector<std::string>>::reserveReplyStorage( GetterReply* reply ) const
{
    reply->mutable_strings()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<std::string>>::addValueToReply( size_t valueIndex, GetterReply* reply ) const
{
    reply->mutable_strings()->add_data( data[valueIndex] );
}

template <>
size_t DataHolder<std::vector<std::string>>::getValuesFromChunk( size_t startIndex, const SetterChunk* chunk )
{
    size_t chunkSize    = chunk->strings().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->strings().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<std::string>>::applyValuesToProxyField( ProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<ProxyValueField<std::vector<std::string>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GetterStateHandler::GetterStateHandler()
    : m_fieldOwner( nullptr )
    , m_proxyField( nullptr )
    , m_currentDataIndex( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GetterStateHandler::init( const MethodRequest* request )
{
    m_fieldOwner = ObjectService::findCafObjectFromRpcObject( request->self() );
    std::vector<FieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<AbstractFieldScriptingCapability>();
        if ( scriptability && request->method() == scriptability->scriptFieldName() )
        {
            ProxyFieldHandle* proxyField = dynamic_cast<ProxyFieldHandle*>( field );
            if ( proxyField )
            {
                m_proxyField = proxyField;

                if ( dynamic_cast<ProxyValueField<std::vector<int>>*>( field ) )
                {
                    auto dataField = dynamic_cast<ProxyValueField<std::vector<int>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<int>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<ProxyValueField<std::vector<double>>*>( field ) )
                {
                    auto dataField = dynamic_cast<ProxyValueField<std::vector<double>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<double>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<ProxyValueField<std::vector<std::string>>*>( field ) )
                {
                    auto dataField = dynamic_cast<ProxyValueField<std::vector<std::string>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<std::string>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else
                {
                    CAF_ASSERT( false && "The proxy field data type is not yet supported for streaming fields" );
                }
            }
        }
    }

    return grpc::Status( grpc::NOT_FOUND, "Proxy field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GetterStateHandler::assignReply( GetterReply* reply )
{
    CAF_ASSERT( m_dataHolder );
    size_t dataUnitsInPackage = ServiceInterface::packageByteSize() / m_dataHolder->valueSizeOf();

    size_t indexInPackage = 0u;
    m_dataHolder->reserveReplyStorage( reply );

    for ( ; indexInPackage < dataUnitsInPackage && m_currentDataIndex < m_dataHolder->valueCount(); ++indexInPackage )
    {
        m_dataHolder->addValueToReply( m_currentDataIndex, reply );
        m_currentDataIndex++;
    }
    if ( indexInPackage > 0u )
    {
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::OUT_OF_RANGE,
                         "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t GetterStateHandler::streamedValueCount() const
{
    return m_currentDataIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t GetterStateHandler::totalValueCount() const
{
    return m_dataHolder->valueCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GetterStateHandler::finish()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StateHandler<MethodRequest>* GetterStateHandler::emptyClone() const
{
    return new GetterStateHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SetterStateHandler::SetterStateHandler()
    : m_fieldOwner( nullptr )
    , m_proxyField( nullptr )
    , m_currentDataIndex( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status SetterStateHandler::init( const SetterChunk* chunk )
{
    CAF_ASSERT( chunk->has_set_request() );
    auto setRequest    = chunk->set_request();
    auto methodRequest = setRequest.request();
    m_fieldOwner       = ObjectService::findCafObjectFromRpcObject( methodRequest.self() );
    int valueCount     = setRequest.value_count();

    std::vector<FieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<AbstractFieldScriptingCapability>();
        if ( scriptability && methodRequest.method() == scriptability->scriptFieldName() )
        {
            ProxyFieldHandle* proxyField = dynamic_cast<ProxyFieldHandle*>( field );
            if ( proxyField )
            {
                m_proxyField = proxyField;

                if ( dynamic_cast<ProxyValueField<std::vector<int>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<int>>( std::vector<int>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<ProxyValueField<std::vector<double>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<double>>( std::vector<double>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<ProxyValueField<std::vector<std::string>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<std::string>>( std::vector<std::string>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else
                {
                    CAF_ASSERT( false && "The proxy field data type is not yet supported for streaming fields" );
                }
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Proxy field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status SetterStateHandler::receiveRequest( const SetterChunk* chunk, SetterReply* reply )
{
    size_t valuesWritten = m_dataHolder->getValuesFromChunk( m_currentDataIndex, chunk );
    m_currentDataIndex += valuesWritten;

    if ( m_currentDataIndex > totalValueCount() )
    {
        return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
    }
    reply->set_value_count( static_cast<int64_t>( m_currentDataIndex ) );
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t SetterStateHandler::streamedValueCount() const
{
    return m_currentDataIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t SetterStateHandler::totalValueCount() const
{
    return m_dataHolder->valueCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SetterStateHandler::finish()
{
    if ( m_proxyField )
    {
        auto scriptingCapability = m_proxyField->capability<AbstractFieldScriptingCapability>();
        CAF_ASSERT( scriptingCapability );
        Variant before = m_proxyField->toVariant();
        m_dataHolder->applyValuesToProxyField( m_proxyField );
        Variant after = m_proxyField->toVariant();
        m_fieldOwner->fieldChangedByCapability( m_proxyField, scriptingCapability, before, after );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StateHandler<SetterChunk>* SetterStateHandler::emptyClone() const
{
    return new SetterStateHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::GetDocument( grpc::ServerContext* context, const DocumentRequest* request, Object* reply )
{
    PdmDocument* document = caf::Application::instance()->document( request->document_id() );
    if ( document )
    {
        copyObjectFromCafToRpc( document, reply );
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Document not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::Sync( grpc::ServerContext* context, const Object* request, Object* reply )
{
    auto matchingObject = findCafObjectFromRpcObject( *request );

    if ( matchingObject )
    {
        copyObjectFromRpcToCaf( request, matchingObject );
        copyObjectFromCafToRpc( matchingObject, reply );
        matchingObject->updateAllRequiredEditors();

        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Object not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteGetter( grpc::ServerContext*         context,
                                           const MethodRequest*         request,
                                           GetterReply*                 reply,
                                           StateHandler<MethodRequest>* stateHandler )
{
    auto getterHandler = dynamic_cast<GetterStateHandler*>( stateHandler );
    CAF_ASSERT( getterHandler );

    return getterHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteSetter( grpc::ServerContext*       context,
                                           const SetterChunk*         chunk,
                                           SetterReply*               reply,
                                           StateHandler<SetterChunk>* stateHandler )
{
    auto setterHandler = dynamic_cast<SetterStateHandler*>( stateHandler );
    CAF_ASSERT( setterHandler );
    return setterHandler->receiveRequest( chunk, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status ObjectService::ExecuteMethod( grpc::ServerContext* context, const MethodRequest* request, Object* reply )
{
    auto matchingObject = findCafObjectFromRpcObject( request->self() );
    if ( matchingObject )
    {
        std::shared_ptr<ObjectMethod> method =
            ObjectMethodFactory::instance()->createMethod( matchingObject, request->method() );
        if ( method )
        {
            copyObjectFromRpcToCaf( &( request->params() ), method.get() );

            ObjectHandle* result = method->execute();
            if ( result )
            {
                copyObjectFromCafToRpc( result, reply );
                if ( !method->resultIsPersistent() )
                {
                    delete result;
                }
                return grpc::Status::OK;
            }
            else
            {
                if ( method->isNullptrValidResult() )
                {
                    return grpc::Status::OK;
                }

                return grpc::Status( grpc::NOT_FOUND, "No result returned from Method" );
            }
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find Method" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find PdmObject" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object* ObjectService::findCafObjectFromRpcObject( const Object& rpcObject )
{
    return findCafObjectFromScriptNameAndAddress( rpcObject.class_keyword(), rpcObject.address() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object* ObjectService::findCafObjectFromScriptNameAndAddress( const std::string& scriptClassName, uint64_t address )
{
    std::list<caf::ObjectHandle*> objectsOfCurrentClass;

    if ( caf::Application::instance()->hasCapability( AppCapability::GRPC_CLIENT ) )
    {
        objectsOfCurrentClass = GrpcClientObjectFactory::instance()->objectsWithClassKeyword( scriptClassName );
    }

    for ( auto doc : caf::Application::instance()->documents() )
    {
        std::vector<caf::Object*> objects;
        doc->descendantsIncludingThisFromClassKeyword( scriptClassName, objects );
        for ( auto object : objects )
        {
            objectsOfCurrentClass.push_back( object );
        }
    }

    caf::Object* matchingObject = nullptr;
    for ( ObjectHandle* testObjectHandle : objectsOfCurrentClass )
    {
        caf::Object* testObject = dynamic_cast<caf::Object*>( testObjectHandle );
        if ( testObject && reinterpret_cast<uint64_t>( testObject ) == address )
        {
            matchingObject = testObject;
        }
    }
    return matchingObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyObjectFromCafToRpc( const caf::ObjectHandle* source, Object* destination )
{
    CAF_ASSERT( source && destination );

    std::stringstream ss;
    caf::ObjectJsonCapability::writeFile( source, ss, true );

    auto ioCapability = source->capability<caf::ObjectIoCapability>();
    CAF_ASSERT( ioCapability );

    destination->set_class_keyword( ioCapability->classKeyword() );
    auto clientCapability = source->capability<caf::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        destination->set_address( clientCapability->addressOnServer() );
    }
    else
    {
        destination->set_address( reinterpret_cast<uint64_t>( source ) );
    }
    destination->set_json( ss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectService::copyObjectFromRpcToCaf( const Object* source, caf::ObjectHandle* destination )
{
    CAF_ASSERT( source );

    auto              ioCapability = destination->capability<caf::ObjectIoCapability>();
    std::stringstream str( source->json() );
    ioCapability->readFile( str );

    auto clientCapability = destination->capability<caf::rpc::ObjectClientCapability>();
    if ( clientCapability )
    {
        clientCapability->setAddressOnServer( source->address() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::ObjectHandle> ObjectService::createCafObjectFromRpc( const Object* source )
{
    CAF_ASSERT( source );
    std::unique_ptr<caf::ObjectHandle> destination(
        caf::ObjectJsonCapability::readUnknownObjectFromString( source->json(), GrpcClientObjectFactory::instance(), false ) );

    return destination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> ObjectService::registerCallbacks()
{
    typedef ObjectService Self;
    return {
        new UnaryCallback<Self, DocumentRequest, Object>( this, &Self::GetDocument, &Self::RequestGetDocument ),
        new UnaryCallback<Self, Object, Object>( this, &Self::Sync, &Self::RequestSync ),
        new ServerToClientStreamCallback<Self, MethodRequest, GetterReply>( this,
                                                                            &Self::ExecuteGetter,
                                                                            &Self::RequestExecuteGetter,
                                                                            new GetterStateHandler ),

        new ClientToServerStreamCallback<Self, SetterChunk, SetterReply>( this,
                                                                          &Self::ExecuteSetter,
                                                                          &Self::RequestExecuteSetter,
                                                                          new SetterStateHandler ),
        new UnaryCallback<Self, MethodRequest, Object>( this, &Self::ExecuteMethod, &Self::RequestExecuteMethod ),

    };
}

static bool ObjectService_init =
    ServiceFactory::instance()->registerCreator<ObjectService>( typeid( ObjectService ).hash_code() );

} // namespace caf::rpc
