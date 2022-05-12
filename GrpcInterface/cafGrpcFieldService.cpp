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
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObject.h"

#include <grpcpp/grpcpp.h>

#include <vector>

namespace caffa::rpc
{
std::map<const caffa::ObjectHandle*, std::map<std::string, caffa::FieldHandle*>> FieldService::s_fieldCache;
std::mutex                                                                       FieldService::s_fieldCacheMutex;

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
    void                  applyValuesToField( FieldHandle* field ) override;
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
void DataHolder<int>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<int>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<uint32_t>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<uint32_t>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<int64_t>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<int64_t>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<uint64_t>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<uint64_t>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<double>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<double>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<float>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<float>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<std::string>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<std::string>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<bool>::applyValuesToField( FieldHandle* field )
{
    auto typedField = dynamic_cast<caffa::Field<std::vector<bool>>*>( field );
    if ( typedField )
    {
        CAFFA_TRACE( "Applying " << data.size() << " values to field" );
        typedField->setValue( data );
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
void DataHolder<ObjectHandle*>::applyValuesToField( FieldHandle* field )
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
    CAFFA_ASSERT( request != nullptr );

    m_fieldOwner = ObjectService::ObjectService::findCafObjectFromFieldRequest( *request );
    CAFFA_ASSERT( m_fieldOwner );
    if ( !m_fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : m_fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && request->keyword() == scriptability->scriptFieldName() )
        {
            if ( !scriptability->isReadable() )
            {
                return grpc::Status( grpc::INVALID_ARGUMENT, "Data field is not readable" );
            }

            if ( auto dataField = dynamic_cast<TypedField<std::vector<int>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<unsigned>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<unsigned>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<int64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<int64_t>( dataField->value() ) );
                return grpc::Status::OK;
            }

            else if ( auto dataField = dynamic_cast<TypedField<std::vector<uint64_t>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<uint64_t>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<double>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<double>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<float>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<float>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<std::string>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::string>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedField<std::vector<bool>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<bool>( dataField->value() ) );
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

    return grpc::Status( grpc::NOT_FOUND, "Field not found: '" + request->keyword() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status GetterStateHandler::assignReply( GenericArray* reply )
{
    CAFFA_ASSERT( m_dataHolder );

    size_t remainingData = m_dataHolder->valueCount() - m_currentDataIndex;

    size_t defaultDataUnitsInPackage = Application::instance()->packageByteSize() / m_dataHolder->valueSizeOf();

    size_t dataUnitsInPackage = std::min( defaultDataUnitsInPackage, remainingData );
    CAFFA_TRACE( "Assigning values " << dataUnitsInPackage << " to getter reply" );

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
//--------------------------------------------------------------------------------------------------set
grpc::Status SetterStateHandler::init( const GenericArray* chunk )
{
    CAFFA_ASSERT( chunk->has_request() );
    auto setRequest = chunk->request();

    auto fieldRequest = setRequest.field();
    m_fieldOwner      = ObjectService::findCafObjectFromFieldRequest( fieldRequest );

    if ( !m_fieldOwner )
    {
        return grpc::Status( grpc::NOT_FOUND,
                             "Could not found the self object " + fieldRequest.class_keyword() + ", " +
                                 fieldRequest.uuid() );
    }

    int valueCount = setRequest.value_count();

    grpc::Status status = grpc::Status::OK;
    if ( valueCount == 0 )
    {
        status = grpc::Status( grpc::OUT_OF_RANGE,
                               "We've reached the end. This is not an error but means transmission is finished" );
    }
    for ( auto field : m_fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && scriptability->scriptFieldName() == fieldRequest.keyword() )
        {
            if ( !scriptability->isWritable() )
            {
                return grpc::Status( grpc::INVALID_ARGUMENT, "Data field is not writable" );
            }

            try
            {
                if ( auto dataField = dynamic_cast<TypedField<std::vector<int>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<int>( std::vector<int>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<unsigned>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<unsigned>( std::vector<unsigned>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<int64_t>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<int64_t>( std::vector<int64_t>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<uint64_t>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<uint64_t>( std::vector<uint64_t>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<double>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<double>( std::vector<double>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<float>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<float>( std::vector<float>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<bool>>*>( field ); dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<bool>( std::vector<bool>( valueCount ) ) );
                    return status;
                }
                else if ( auto dataField = dynamic_cast<TypedField<std::vector<std::string>>*>( field );
                          dataField != nullptr )
                {
                    m_field = dataField;
                    m_dataHolder.reset( new DataHolder<std::string>( std::vector<std::string>( valueCount ) ) );
                    return status;
                }
                else
                {
                    return grpc::Status( grpc::UNIMPLEMENTED, "Data type not implemented for grpc streaming fields" );
                }
            }
            catch ( const std::exception& e )
            {
                CAFFA_ERROR( "SetArrayValue for '" << fieldRequest.keyword() << "' failed with error '" << e.what() << "'" );
                return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found: '" + fieldRequest.keyword() + "'" );
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
    CAFFA_ASSERT( m_field && "Field has to be set for any writing to work" );

    auto scriptingCapability = m_field->capability<caffa::FieldScriptingCapability>();
    CAFFA_ASSERT( scriptingCapability );
    m_dataHolder->applyValuesToField( m_field );
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
grpc::Status FieldService::GetArrayValueWithState( grpc::ServerContext*        context,
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
    CAFFA_ASSERT( request != nullptr );
    CAFFA_TRACE( "GetValue for field: " << request->keyword() << ", " << request->class_keyword() << ", "
                                        << request->uuid() );

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromFieldRequest( *request );
    CAFFA_ASSERT( fieldOwner );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    auto field = scriptableFieldFromKeyword( fieldOwner, request->keyword() );

    if ( field )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        CAFFA_ASSERT( scriptability );
        if ( scriptability->isReadable() )
        {
            bool isObjectField = dynamic_cast<caffa::ChildFieldHandle*>( field ) != nullptr;
            auto ioCapability  = field->capability<caffa::FieldJsonCapability>();
            if ( ioCapability )
            {
                try
                {
                    nlohmann::json jsonValue;
                    JsonSerializer serializer;
                    serializer.setSerializeDataValues( !isObjectField );
                    ioCapability->writeToJson( jsonValue, serializer );
                    if ( jsonValue.is_object() && jsonValue.contains( "value" ) )
                    {
                        reply->set_value( jsonValue["value"].dump() );
                    }
                    else
                    {
                        reply->set_value( jsonValue.is_null() ? "" : jsonValue.dump() );
                    }
                    CAFFA_TRACE( "Get " << fieldOwner->classKeyword() << " -> " << field->keyword() << " = "
                                        << reply->value() );

                    return grpc::Status::OK;
                }
                catch ( const std::exception& e )
                {
                    CAFFA_ERROR( "GetValue for '" << request->keyword() << "' failed with error: '" << e.what() << "'" );
                    return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
                }
            }
        }
        return grpc::Status( grpc::FAILED_PRECONDITION, "Field " + request->keyword() + " found, but it isn't readable" );
    }
    std::string errMsg = std::string( "Field " ) + request->keyword() + " not found in " + fieldOwner->classKeyword();
    CAFFA_ERROR( errMsg );
    return grpc::Status( grpc::NOT_FOUND, errMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::ClearChildObjects( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply )
{
    CAFFA_ASSERT( request != nullptr );
    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromFieldRequest( *request );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( "Clear Child Objects for field: " << request->keyword() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && request->keyword() == scriptability->scriptFieldName() )
        {
            try
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
            catch ( const std::exception& e )
            {
                CAFFA_ERROR( "ClearChildObjects for '" << request->keyword() << "' failed with error: '" << e.what()
                                                       << "'" );
                return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found: '" + request->keyword() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::RemoveChildObject( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply )
{
    CAFFA_ASSERT( request != nullptr );
    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromFieldRequest( *request );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( "Remove Child Object at index " << request->index() << " for field: " << request->keyword() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && request->keyword() == scriptability->scriptFieldName() )
        {
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
            if ( childArrayField )
            {
                try
                {
                    childArrayField->erase( request->index() );
                    return grpc::Status::OK;
                }
                catch ( const std::exception& e )
                {
                    CAFFA_ERROR( "RemoveChildObjects for '" << request->keyword() << "' failed with error: '"
                                                            << e.what() << "'" );
                    return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
                }
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found: '" + request->keyword() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::InsertChildObject( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply )
{
    auto fieldRequest = request->field();

    auto fieldOwner = ObjectService::findCafObjectFromFieldRequest( fieldRequest );
    CAFFA_ASSERT( fieldOwner );
    CAFFA_TRACE( " Inserting Child Object at index " << fieldRequest.index() << " for field: " << fieldRequest.keyword() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && fieldRequest.keyword() == scriptability->scriptFieldName() )
        {
            auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
            if ( childArrayField )
            {
                try
                {
                    std::unique_ptr<caffa::ObjectHandle> newCafObject =
                        caffa::JsonSerializer( DefaultObjectFactory::instance() ).createObjectFromString( request->value() );
                    if ( !newCafObject ) throw std::runtime_error( "Failed to create new caf object" );
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
                catch ( const std::exception& e )
                {
                    CAFFA_ERROR( "InsertChildObjects for '" << fieldRequest.keyword() << "' failed with " << e.what() );
                    return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
                }
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found: '" + fieldRequest.keyword() + "'" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::SetArrayValueWithState( grpc::ServerContext*        context,
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
    auto fieldOwner   = ObjectService::ObjectService::findCafObjectFromFieldRequest( fieldRequest );
    CAFFA_ASSERT( fieldOwner );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    CAFFA_TRACE( "Received Set Request for class " << fieldOwner->classKeyword() );

    auto field = scriptableFieldFromKeyword( fieldOwner, fieldRequest.keyword() );

    if ( field )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        CAFFA_ASSERT( scriptability );

        auto ioCapability = field->capability<caffa::FieldJsonCapability>();
        try
        {
            if ( !scriptability->isWritable() || !ioCapability )
                throw std::runtime_error( "Field " + fieldRequest.keyword() + " is not writable" );

            CAFFA_TRACE( "Set " << fieldOwner->classKeyword() << " -> " << fieldRequest.keyword() << " = "
                                << request->value() << "" );

            auto           jsonValue = nlohmann::json::parse( request->value() );
            JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
            ioCapability->readFromJson( jsonValue, serializer );
            return grpc::Status::OK;
        }
        catch ( const std::exception& e )
        {
            CAFFA_ERROR( "SetValue for '" << fieldRequest.keyword() << "' failed with error: '" << e.what() << "'" );
            return grpc::Status( grpc::FAILED_PRECONDITION, e.what() );
        }
    }
    std::string errMsg = std::string( "Field " ) + fieldRequest.keyword() + " not found in " + fieldOwner->classKeyword();
    CAFFA_ERROR( errMsg );
    return grpc::Status( grpc::NOT_FOUND, errMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> FieldService::createCallbacks()
{
    typedef FieldService Self;
    return { new ServerToClientStreamCallback<Self, FieldRequest, GenericArray>( this,
                                                                                 &Self::GetArrayValueWithState,
                                                                                 &Self::RequestGetArrayValue,
                                                                                 new GetterStateHandler ),
             new ClientToServerStreamCallback<Self, GenericArray, SetterArrayReply>( this,
                                                                                     &Self::SetArrayValueWithState,
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

caffa::FieldHandle* FieldService::scriptableFieldFromKeyword( const caffa::ObjectHandle* fieldOwner,
                                                              const std::string&         keyword )
{
    CAFFA_ASSERT( fieldOwner );

    caffa::FieldHandle* field = nullptr;
    {
        std::scoped_lock cacheLock( s_fieldCacheMutex );
        auto             it = s_fieldCache.find( fieldOwner );
        if ( it != s_fieldCache.end() )
        {
            auto jt = it->second.find( keyword );
            if ( jt != it->second.end() )
            {
                field = jt->second;
            }
        }
    }

    if ( !field )
    {
        for ( auto tryField : fieldOwner->fields() )
        {
            if ( tryField->keyword() == keyword )
            {
                field = tryField;

                std::scoped_lock cacheLock( s_fieldCacheMutex );
                s_fieldCache[fieldOwner].insert( std::make_pair( keyword, field ) );
                break;
            }
        }
    }

    if ( field )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && keyword == scriptability->scriptFieldName() )
        {
            return field;
        }
    }
    return nullptr;
}

} // namespace caffa::rpc
