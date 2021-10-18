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
#include "cafGrpcFieldService.h"

#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafGrpcApplication.h"
#include "cafGrpcCallbacks.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcObjectService.h"
#include "cafGrpcServerApplication.h"
#include "cafLogger.h"
#include "cafObject.h"

#include <grpcpp/grpcpp.h>

#include <variant>
#include <vector>

namespace caffa::rpc
{
template <typename DataType>
struct DataHolder : public AbstractDataHolder
{
    DataHolder( const std::vector<DataType>& data )
        : data( data )
    {
    }

    size_t valueCount() const override;
    size_t valueSizeOf() const override;

    void reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const override
    {
        static_assert( "Should never be called!" );
    }
    void addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const override
    {
        static_assert( "Should never be called!" );
    }

    size_t                getValuesFromChunk( size_t startIndex, const GenericArray* chunk ) override;
    void                  applyValuesToField( ValueField* field ) override;
    std::vector<DataType> data;
};

template <>
size_t DataHolder<int>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<int>::valueSizeOf() const
{
    return sizeof( int );
}

template <>
void DataHolder<int>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_ints()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<int>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_ints()->mutable_data() ) = { data.begin() + startIndex,
                                                   data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<int>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
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
void DataHolder<int>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<int>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<uint32_t>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<uint32_t>::valueSizeOf() const
{
    return sizeof( uint32_t );
}

template <>
void DataHolder<uint32_t>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_uints()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<uint32_t>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_uints()->mutable_data() ) = { data.begin() + startIndex,
                                                    data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<uint32_t>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = chunk->uints().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->uints().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<uint32_t>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<uint32_t>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<int64_t>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<int64_t>::valueSizeOf() const
{
    return sizeof( int64_t );
}

template <>
void DataHolder<int64_t>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_int64s()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<int64_t>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_int64s()->mutable_data() ) = { data.begin() + startIndex,
                                                     data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<int64_t>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = chunk->int64s().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->int64s().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<int64_t>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<int64_t>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<uint64_t>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<uint64_t>::valueSizeOf() const
{
    return sizeof( uint64_t );
}

template <>
void DataHolder<uint64_t>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_uint64s()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<uint64_t>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_uint64s()->mutable_data() ) = { data.begin() + startIndex,
                                                      data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<uint64_t>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = chunk->uint64s().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->uint64s().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<uint64_t>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<uint64_t>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<double>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<double>::valueSizeOf() const
{
    return sizeof( double );
}

template <>
void DataHolder<double>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_doubles()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<double>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_doubles()->mutable_data() ) = { data.begin() + startIndex,
                                                      data.begin() + startIndex + numberOfDataUnits };
}
template <>
size_t DataHolder<double>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
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
void DataHolder<double>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<double>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}
template <>
size_t DataHolder<float>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<float>::valueSizeOf() const
{
    return sizeof( float );
}

template <>
void DataHolder<float>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_floats()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<float>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_floats()->mutable_data() ) = { data.begin() + startIndex,
                                                     data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<float>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = chunk->floats().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->floats().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<float>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<float>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<std::string>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<std::string>::valueSizeOf() const
{
    return sizeof( std::string );
}

template <>
void DataHolder<std::string>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_strings()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<std::string>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_strings()->mutable_data() ) = { data.begin() + startIndex,
                                                      data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<std::string>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = 1u;
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->strings().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::string>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<std::string>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<bool>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<bool>::valueSizeOf() const
{
    return sizeof( bool );
}

template <>
void DataHolder<bool>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    reply->mutable_bools()->mutable_data()->Reserve( numberOfDataUnits );
}

template <>
void DataHolder<bool>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    *( reply->mutable_bools()->mutable_data() ) = { data.begin() + startIndex,
                                                    data.begin() + startIndex + numberOfDataUnits };
}

template <>
size_t DataHolder<bool>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    size_t chunkSize    = chunk->bools().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->bools().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<bool>::applyValuesToField( ValueField* field )
{
    auto dataValueField = dynamic_cast<Field<std::vector<bool>>*>( field );
    if ( dataValueField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        dataValueField->setValueWithFieldChanged( data, dataValueField->capability<FieldScriptingCapability>() );
    }
}

template <>
size_t DataHolder<ObjectHandle*>::valueCount() const
{
    return data.size();
}
template <>
size_t DataHolder<ObjectHandle*>::valueSizeOf() const
{
    return 1;
}

template <>
void DataHolder<ObjectHandle*>::reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const
{
    // Do nothing
}

template <>
void DataHolder<ObjectHandle*>::addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const
{
    for ( size_t i = 0; i < numberOfDataUnits; ++i )
    {
        auto          it           = data.begin() + startIndex + i;
        RpcObject*    rpcObject    = reply->mutable_objects()->add_objects();
        ObjectHandle* objectHandle = *it;
        ObjectService::copyProjectObjectFromCafToRpc( objectHandle, rpcObject );
    }
}

template <>
size_t DataHolder<ObjectHandle*>::getValuesFromChunk( size_t startIndex, const GenericArray* chunk )
{
    CAFFA_ASSERT( false && "Not implemented" );
}
template <>
void DataHolder<ObjectHandle*>::applyValuesToField( ValueField* field )
{
    CAFFA_ASSERT( false && "Not implemented" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GetterStateHandler::GetterStateHandler()
    : m_fieldOwner( nullptr )
    , m_field( nullptr )
    , m_currentDataIndex( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GetterStateHandler::init( const FieldRequest* request )
{
    m_fieldOwner = ObjectService::findCafObjectFromRpcObject( request->self() );
    CAFFA_ASSERT( m_fieldOwner );
    if ( !m_fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : m_fieldOwner->fields() )
    {
        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && request->method() == scriptability->scriptFieldName() )
        {
            if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<unsigned>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<unsigned>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int64_t>( dataField->value() ) );
                return grpc::Status::OK;
            }

            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<uint64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<uint64_t>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<double>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<double>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<float>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<float>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<std::string>>*>( field );
                      dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::string>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto objectField = dynamic_cast<ChildArrayFieldHandle*>( field ); objectField != nullptr )
            {
                m_childArrayField = objectField;
                m_dataHolder.reset( new DataHolder<ObjectHandle*>( objectField->childObjects() ) );
                return grpc::Status::OK;
            }
            else
            {
                return grpc::Status( grpc::UNIMPLEMENTED, "Data type not implemented for grpc streaming fields" );
            }
        }
    }

    return grpc::Status( grpc::NOT_FOUND, "Field not found " + request->method() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GetterStateHandler::assignReply( GenericArray* reply )
{
    CAFFA_ASSERT( m_dataHolder );

    size_t remainingData = m_dataHolder->valueCount() - m_currentDataIndex;

    CAFFA_TRACE( "Assigning values " << remainingData << " to getter reply" );

    size_t defaultDataUnitsInPackage = Application::instance()->packageByteSize() / m_dataHolder->valueSizeOf();

    size_t dataUnitsInPackage = std::min( defaultDataUnitsInPackage, remainingData );
    if ( dataUnitsInPackage == 0u )
    {
        return grpc::Status( grpc::OUT_OF_RANGE,
                             "We've reached the end. This is not an error but means transmission is finished" );
    }
    m_dataHolder->addPackageValuesToReply( reply, m_currentDataIndex, dataUnitsInPackage );
    m_currentDataIndex += dataUnitsInPackage;
    CAFFA_TRACE( "Sending " << dataUnitsInPackage << " values" );
    return grpc::Status::OK;
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
StateHandler<FieldRequest>* GetterStateHandler::emptyClone() const
{
    return new GetterStateHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SetterStateHandler::SetterStateHandler()
    : m_fieldOwner( nullptr )
    , m_field( nullptr )
    , m_currentDataIndex( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status SetterStateHandler::init( const GenericArray* chunk )
{
    CAFFA_ASSERT( chunk->has_request() );
    auto setRequest = chunk->request();

    auto fieldRequest = setRequest.field();
    m_fieldOwner      = ObjectService::findCafObjectFromRpcObject( fieldRequest.self() );
    int valueCount    = setRequest.value_count();

    for ( auto field : m_fieldOwner->fields() )
    {
        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && scriptability->scriptFieldName() == fieldRequest.method() )
        {
            if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int>( std::vector<int>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<unsigned>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<unsigned>( std::vector<unsigned>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int64_t>( std::vector<int64_t>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<uint64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<uint64_t>( std::vector<uint64_t>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<double>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<double>( std::vector<double>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<float>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<float>( std::vector<float>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<std::string>>*>( field );
                      dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::string>( std::vector<std::string>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else
            {
                return grpc::Status( grpc::UNIMPLEMENTED, "Data type not implemented for grpc streaming fields" );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status SetterStateHandler::receiveRequest( const GenericArray* chunk, SetterArrayReply* reply )
{
    CAFFA_TRACE( "Received Setter Chunk" );

    size_t valuesWritten = m_dataHolder->getValuesFromChunk( m_currentDataIndex, chunk );
    m_currentDataIndex += valuesWritten;

    if ( m_currentDataIndex > totalValueCount() )
    {
        return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
    }
    reply->set_value_count( static_cast<int64_t>( m_currentDataIndex ) );
    CAFFA_TRACE( "Received " << reply->value_count() << " values" );
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
    if ( m_field )
    {
        auto scriptingCapability = m_field->capability<FieldScriptingCapability>();
        CAFFA_ASSERT( scriptingCapability );
        Variant before = m_field->toVariant();
        m_dataHolder->applyValuesToField( m_field );
        Variant after = m_field->toVariant();
        m_fieldOwner->fieldChangedByCapability( m_field, scriptingCapability, before, after );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StateHandler<GenericArray>* SetterStateHandler::emptyClone() const
{
    return new SetterStateHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::GetArrayValue( grpc::ServerContext*        context,
                                          const FieldRequest*         request,
                                          GenericArray*               reply,
                                          StateHandler<FieldRequest>* stateHandler )
{
    CAFFA_TRACE( "Received GetArrayValue request" );
    auto getterHandler = dynamic_cast<GetterStateHandler*>( stateHandler );
    CAFFA_ASSERT( getterHandler );
    return getterHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::GetValue( grpc::ServerContext* context, const FieldRequest* request, GenericScalar* reply )
{
    auto fieldOwner = ObjectService::findCafObjectFromRpcObject( request->self() );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( "GetValue for field: " << request->method() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    bool foundMatchingField = false;
    for ( auto field : fieldOwner->fields() )
    {
        bool isObjectField = dynamic_cast<caffa::ChildFieldHandle*>( field ) != nullptr;

        if ( field->keyword() == request->method() ) foundMatchingField = true;

        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && request->method() == scriptability->scriptFieldName() )
        {
            auto ioCapability = field->capability<caffa::FieldIoCapability>();
            if ( ioCapability )
            {
                nlohmann::json jsonValue;
                ioCapability->writeToJson( jsonValue, !isObjectField );
                reply->set_value( jsonValue.is_null() ? "" : jsonValue.dump() );
                CAFFA_DEBUG( "Sending json value: '" << reply->value() << "'" );

                return grpc::Status::OK;
            }
        }
    }

    if ( foundMatchingField )
    {
        return grpc::Status( grpc::FAILED_PRECONDITION,
                             "Field " + request->method() +
                                 " found, but it either isn't scriptable or does not have I/O capability" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::ClearChildObjects( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply )
{
    auto fieldOwner = ObjectService::findCafObjectFromRpcObject( request->self() );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( "Clear Child Objects for field: " << request->method() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && request->method() == scriptability->scriptFieldName() )
        {
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
            if ( childArrayField )
            {
                childArrayField->clear();
                return grpc::Status::OK;
            }

            auto childField = dynamic_cast<ChildFieldHandle*>( field );
            if ( childField )
            {
                childField->clear();
                return grpc::Status::OK;
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::RemoveChildObject( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply )
{
    auto fieldOwner = ObjectService::findCafObjectFromRpcObject( request->self() );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( "Remove Child Object at index " << request->index() << " for field: " << request->method() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && request->method() == scriptability->scriptFieldName() )
        {
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
            if ( childArrayField )
            {
                childArrayField->erase( request->index() );
                return grpc::Status::OK;
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::InsertChildObject( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply )
{
    auto fieldRequest = request->field();

    auto fieldOwner = ObjectService::findCafObjectFromRpcObject( fieldRequest.self() );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( " Inserting Child Object at index " << fieldRequest.index() << " for field: " << fieldRequest.method() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && fieldRequest.method() == scriptability->scriptFieldName() )
        {
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
            if ( childArrayField )
            {
                std::unique_ptr<caffa::ObjectHandle> newCafObject =
                    caffa::ObjectJsonCapability::readUnknownObjectFromString( request->value(),
                                                                              DefaultObjectFactory::instance(),
                                                                              true );
                size_t index = fieldRequest.index();
                if ( index >= childArrayField->size() )
                {
                    childArrayField->push_back_obj( std::move( newCafObject ) );
                }
                else
                {
                    childArrayField->insertAt( index, std::move( newCafObject ) );
                }
                return grpc::Status::OK;
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::SetArrayValue( grpc::ServerContext*        context,
                                          const GenericArray*         chunk,
                                          SetterArrayReply*           reply,
                                          StateHandler<GenericArray>* stateHandler )
{
    auto setterHandler = dynamic_cast<SetterStateHandler*>( stateHandler );
    CAFFA_ASSERT( setterHandler );
    return setterHandler->receiveRequest( chunk, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::SetValue( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply )
{
    auto fieldRequest = request->field();
    auto fieldOwner   = ObjectService::findCafObjectFromRpcObject( fieldRequest.self() );
    CAFFA_ASSERT( fieldOwner );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    CAFFA_DEBUG( "Received Set Request for class " << fieldOwner->classKeyword() );

    bool foundMatchingField = false;
    for ( auto field : fieldOwner->fields() )
    {
        if ( field->keyword() == fieldRequest.method() ) foundMatchingField = true;

        auto scriptability = field->capability<FieldScriptingCapability>();
        if ( scriptability && fieldRequest.method() == scriptability->scriptFieldName() )
        {
            CAFFA_DEBUG( "   With value: '" << request->value() << "'" );
            auto ioCapability = field->capability<caffa::FieldIoCapability>();
            if ( ioCapability )
            {
                auto jsonValue = nlohmann::json::parse( request->value() );
                ioCapability->readFromJson( jsonValue, caffa::DefaultObjectFactory::instance(), true );
                return grpc::Status::OK;
            }
        }
    }

    if ( foundMatchingField )
    {
        return grpc::Status( grpc::FAILED_PRECONDITION,
                             "Field " + fieldRequest.method() +
                                 " found, but it either isn't scriptable or does not have I/O capability" );
    }

    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> FieldService::createCallbacks()
{
    typedef FieldService Self;
    return { new ServerToClientStreamCallback<Self, FieldRequest, GenericArray>( this,
                                                                                 &Self::GetArrayValue,
                                                                                 &Self::RequestGetArrayValue,
                                                                                 new GetterStateHandler ),
             new ClientToServerStreamCallback<Self, GenericArray, SetterArrayReply>( this,
                                                                                     &Self::SetArrayValue,
                                                                                     &Self::RequestSetArrayValue,
                                                                                     new SetterStateHandler ),
             new UnaryCallback<Self, FieldRequest, GenericScalar>( this, &Self::GetValue, &Self::RequestGetValue ),
             new UnaryCallback<Self, SetterRequest, NullMessage>( this, &Self::SetValue, &Self::RequestSetValue ),
             new UnaryCallback<Self, FieldRequest, NullMessage>( this,
                                                                 &Self::ClearChildObjects,
                                                                 &Self::RequestClearChildObjects ),
             new UnaryCallback<Self, FieldRequest, NullMessage>( this,
                                                                 &Self::RemoveChildObject,
                                                                 &Self::RequestRemoveChildObject ),
             new UnaryCallback<Self, SetterRequest, NullMessage>( this,
                                                                  &Self::InsertChildObject,
                                                                  &Self::RequestInsertChildObject ) };
}

} // namespace caffa::rpc
