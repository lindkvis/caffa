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

#include "cafGrpcCallbacks.h"
#include "cafGrpcFieldService.h"
#include "cafGrpcServerApplication.h"

#include "cafAbstractFieldScriptingCapability.h"
#include "cafField.h"
#include "cafGrpcApplication.h"
#include "cafGrpcObjectService.h"
#include "cafObject.h"
#include "cafPdmScriptIOMessages.h"
#include "cafProxyValueField.h"

#include <grpcpp/grpcpp.h>

#include <variant>
#include <vector>

namespace caf::rpc
{
template <typename DataType, typename DataTypeContainer>
struct DataHolder : public AbstractDataHolder
{
    DataHolder( const DataType& data )
        : data( data )
    {
    }

    size_t valueCount() const override { return data.size(); }

    size_t valueSizeOf() const override { return sizeof( DataType ); }

    void addPackageValuesToReply( GetterReplyT* reply, size_t startIndex, size_t numberOfDataUnits ) const override;

    size_t getValuesFromChunk( size_t startIndex, const SetterChunkT* chunk ) override
    {
        auto   array        = chunk->GetRoot()->data_as<DataTypeContainer>();
        auto   vector       = array->data();
        size_t chunkSize    = vector->size();
        size_t currentIndex = startIndex;
        size_t chunkIndex   = 0u;
        for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
        {
            data[currentIndex] = ( *vector )[chunkIndex];
        }
        return chunkSize;
    }
    void applyValuesToField( ValueField* field ) override
    {
        auto proxyValueField = dynamic_cast<ProxyValueField<DataType>*>( field );
        auto dataValueField  = dynamic_cast<Field<DataType>*>( field );
        if ( proxyValueField )
        {
            proxyValueField->setValue( data );
        }
        else if ( dataValueField )
        {
            dataValueField->setValueWithFieldChanged( data );
        }
    }

    DataType data;
};

template <>
void DataHolder<std::vector<int>, IntArray>::addPackageValuesToReply( GetterReplyT* reply,
                                                                      size_t        startIndex,
                                                                      size_t        numberOfDataUnits ) const
{
    flatbuffers::grpc::MessageBuilder mb;

    const int* dataPtr = &data[startIndex];
    auto       vector  = mb.CreateVector<int>( dataPtr, numberOfDataUnits );
    auto       array   = CreateIntArray( mb, vector );

    CreateGetterReply( mb, AnyGetterValueTraits<IntArray>::enum_value, array.Union() );
    *reply = mb.ReleaseMessage<GetterReply>();
}

template <>
void DataHolder<std::vector<double>, DoubleArray>::addPackageValuesToReply( GetterReplyT* reply,
                                                                            size_t        startIndex,
                                                                            size_t        numberOfDataUnits ) const
{
    flatbuffers::grpc::MessageBuilder mb;

    const double* dataPtr = &data[startIndex];
    auto          vector  = mb.CreateVector<double>( dataPtr, numberOfDataUnits );
    auto          array   = CreateDoubleArray( mb, vector );

    CreateGetterReply( mb, AnyGetterValueTraits<DoubleArray>::enum_value, array.Union() );
    *reply = mb.ReleaseMessage<GetterReply>();
}
template <>
void DataHolder<std::vector<float>, FloatArray>::addPackageValuesToReply( GetterReplyT* reply,
                                                                          size_t        startIndex,
                                                                          size_t        numberOfDataUnits ) const
{
    flatbuffers::grpc::MessageBuilder mb;

    const float* dataPtr = &data[startIndex];
    auto         vector  = mb.CreateVector<float>( dataPtr, numberOfDataUnits );
    auto         array   = CreateFloatArray( mb, vector );

    CreateGetterReply( mb, AnyGetterValueTraits<FloatArray>::enum_value, array.Union() );
    *reply = mb.ReleaseMessage<GetterReply>();
}

template <>
void DataHolder<std::vector<std::string>, StringArray>::addPackageValuesToReply( GetterReplyT* reply,
                                                                                 size_t        startIndex,
                                                                                 size_t        numberOfDataUnits ) const
{
}

template <>
size_t DataHolder<std::vector<std::string>, StringArray>::getValuesFromChunk( size_t startIndex, const SetterChunkT* chunk )
{
    return 0u;
}

template <>
size_t DataHolder<std::string, Scalar>::valueCount() const
{
    return 1u;
}
template <>
size_t DataHolder<std::string, Scalar>::valueSizeOf() const
{
    return sizeof( std::string );
}

template <>
void DataHolder<std::string, Scalar>::addPackageValuesToReply( GetterReplyT* reply,
                                                               size_t        startIndex,
                                                               size_t        numberOfDataUnits ) const
{
}

template <>
size_t DataHolder<std::string, Scalar>::getValuesFromChunk( size_t startIndex, const SetterChunkT* chunk )
{
    auto scalar = chunk->GetRoot()->data_as_Scalar();
    this->data  = scalar->data()->str();
    return 1u;
}
template <>
void DataHolder<std::string, Scalar>::applyValuesToField( ValueField* field )
{
    auto ioCapability = field->capability<FieldIoCapability>();
    CAF_ASSERT( ioCapability );
    ioCapability->readFieldData( data, caf::DefaultObjectFactory::instance() );
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
grpc::Status GetterStateHandler::init( const FieldRequestT* request )
{
    auto fieldRequest = request->GetRoot();
    m_fieldOwner      = ObjectService::findCafObjectFromRpcObject( *fieldRequest->self() );
    CAF_ASSERT( m_fieldOwner );
    if ( !m_fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );
    std::vector<FieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<AbstractFieldScriptingCapability>();
        if ( scriptability && fieldRequest->method()->str() == scriptability->scriptFieldName() )
        {
            if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<int>, IntArray>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<double>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<double>, DoubleArray>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<float>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<float>, FloatArray>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<std::string>>*>( field );
                      dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<std::string>, StringArray>( dataField->value() ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<ValueField*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                nlohmann::json jsonValue;
                auto           ioCapability = field->capability<caf::FieldIoCapability>();
                ioCapability->writeFieldData( jsonValue, true, true );
                m_dataHolder.reset( new DataHolder<std::string, Scalar>( jsonValue.dump() ) );
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
grpc::Status GetterStateHandler::assignReply( GetterReplyT* reply )
{
    CAF_ASSERT( m_dataHolder );

    size_t remainingData             = m_dataHolder->valueCount() - m_currentDataIndex;
    size_t defaultDataUnitsInPackage = Application::instance()->packageByteSize() / m_dataHolder->valueSizeOf();

    size_t dataUnitsInPackage = std::min( defaultDataUnitsInPackage, remainingData );
    if ( dataUnitsInPackage == 0u )
    {
        return grpc::Status( grpc::OUT_OF_RANGE,
                             "We've reached the end. This is not an error but means transmission is finished" );
    }
    m_dataHolder->addPackageValuesToReply( reply, m_currentDataIndex, dataUnitsInPackage );
    m_currentDataIndex += dataUnitsInPackage;
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
StateHandler<FieldRequestT>* GetterStateHandler::emptyClone() const
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
grpc::Status SetterStateHandler::init( const SetterChunkT* chunk )
{
    auto request = chunk->GetRoot();
    CAF_ASSERT( request->data_type() == AnySetterValue_SetterRequest );

    auto setRequest   = request->data_as_SetterRequest();
    auto fieldRequest = setRequest->request();
    m_fieldOwner      = ObjectService::findCafObjectFromRpcObject( *fieldRequest->self() );
    int valueCount    = setRequest->value_count();

    std::vector<FieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<AbstractFieldScriptingCapability>();
        if ( scriptability && fieldRequest->method()->str() == scriptability->scriptFieldName() )
        {
            if ( auto dataField = dynamic_cast<TypedValueField<std::vector<int>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<int>, IntArray>( std::vector<int>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<double>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<double>, DoubleArray>( std::vector<double>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<float>>*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::vector<float>, FloatArray>( std::vector<float>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<TypedValueField<std::vector<std::string>>*>( field );
                      dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset(
                    new DataHolder<std::vector<std::string>, StringArray>( std::vector<std::string>( valueCount ) ) );
                return grpc::Status::OK;
            }
            else if ( auto dataField = dynamic_cast<ValueField*>( field ); dataField != nullptr )
            {
                m_field = dataField;
                m_dataHolder.reset( new DataHolder<std::string, Scalar>( std::string() ) );
                return grpc::Status::OK;
            }
            else
            {
                return grpc::Status( grpc::UNIMPLEMENTED, "Data type not implemented for grpc streaming fields" );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Proxy field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status SetterStateHandler::receiveRequest( const SetterChunkT* chunk, SetterReplyT* reply )
{
    size_t valuesWritten = m_dataHolder->getValuesFromChunk( m_currentDataIndex, chunk );
    m_currentDataIndex += valuesWritten;

    if ( m_currentDataIndex > totalValueCount() )
    {
        return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
    }

    flatbuffers::grpc::MessageBuilder mb;

    CreateSetterReply( mb, static_cast<int64_t>( m_currentDataIndex ) );
    *reply = mb.ReleaseMessage<SetterReply>();
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
        auto scriptingCapability = m_field->capability<AbstractFieldScriptingCapability>();
        CAF_ASSERT( scriptingCapability );
        Variant before = m_field->toVariant();
        m_dataHolder->applyValuesToField( m_field );
        Variant after = m_field->toVariant();
        m_fieldOwner->fieldChangedByCapability( m_field, scriptingCapability, before, after );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
StateHandler<SetterChunkT>* SetterStateHandler::emptyClone() const
{
    return new SetterStateHandler;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::GetValue( grpc::ServerContext*         context,
                                     const FieldRequestT*         request,
                                     GetterReplyT*                reply,
                                     StateHandler<FieldRequestT>* stateHandler )
{
    auto getterHandler = dynamic_cast<GetterStateHandler*>( stateHandler );
    CAF_ASSERT( getterHandler );
    return getterHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::SetValue( grpc::ServerContext*        context,
                                     const SetterChunkT*         chunk,
                                     SetterReplyT*               reply,
                                     StateHandler<SetterChunkT>* stateHandler )
{
    auto setterHandler = dynamic_cast<SetterStateHandler*>( stateHandler );
    CAF_ASSERT( setterHandler );
    return setterHandler->receiveRequest( chunk, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> FieldService::registerCallbacks()
{
    typedef FieldService Self;
    return {
        new ServerToClientStreamCallback<Self, FieldRequestT, GetterReplyT>( this,
                                                                             &Self::GetValue,
                                                                             &Self::RequestGetValue,
                                                                             new GetterStateHandler ),
        new ClientToServerStreamCallback<Self, SetterChunkT, SetterReplyT>( this,
                                                                            &Self::SetValue,
                                                                            &Self::RequestSetValue,
                                                                            new SetterStateHandler ),
    };
}

} // namespace caf::rpc
