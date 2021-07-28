//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
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
#pragma once

#include "cafGrpcCallbacks.h"
#include "cafGrpcServiceInterface.h"
#include "cafVariant.h"

#include "FieldService.grpc.pb.h"
#include "FieldService.pb.h"

#include <string>
#include <vector>

namespace caffa
{
class ChildFieldHandle;
class FieldHandle;
class Object;
class ObjectFactory;
class ObjectHandle;
class ProxyFieldHandle;
class ValueField;
} // namespace caffa

namespace caffa::rpc
{
class GenericArray;
class GetterReply;
class FieldRequest;
class GenericArray;
class SetterArrayReply;
class SetterReply;

struct AbstractDataHolder
{
    virtual size_t valueCount() const                                                                              = 0;
    virtual size_t valueSizeOf() const                                                                             = 0;
    virtual void   reserveReplyStorage( GenericArray* reply, size_t numberOfDataUnits ) const                      = 0;
    virtual void addPackageValuesToReply( GenericArray* reply, size_t startIndex, size_t numberOfDataUnits ) const = 0;

    virtual size_t getValuesFromChunk( size_t startIndex, const GenericArray* chunk ) = 0;
    virtual void   applyValuesToField( caffa::ValueField* field )                     = 0;
};

/**
 *
 * State handler for client to server streaming
 *
 */
class GetterStateHandler : public StateHandler<FieldRequest>
{
public:
    GetterStateHandler();

    grpc::Status init( const FieldRequest* request ) override;
    grpc::Status assignReply( GenericArray* reply );
    size_t       streamedValueCount() const override;
    size_t       totalValueCount() const override;
    void         finish() override;

    StateHandler<FieldRequest>* emptyClone() const override;

protected:
    caffa::Object*                      m_fieldOwner;
    caffa::ValueField*                  m_field;
    std::unique_ptr<AbstractDataHolder> m_dataHolder;
    size_t                              m_currentDataIndex;
};

/**
 *
 * State handler for client to server streaming
 *
 */
class SetterStateHandler : public StateHandler<GenericArray>
{
public:
    SetterStateHandler();

    grpc::Status init( const GenericArray* chunk ) override;
    grpc::Status receiveRequest( const GenericArray* chunk, SetterArrayReply* reply );
    size_t       streamedValueCount() const override;
    size_t       totalValueCount() const override;
    void         finish() override;

    StateHandler<GenericArray>* emptyClone() const override;

protected:
    caffa::Object*                      m_fieldOwner;
    caffa::ValueField*                  m_field;
    std::unique_ptr<AbstractDataHolder> m_dataHolder;
    size_t                              m_currentDataIndex;
};

//==================================================================================================
//
// gRPC-service answering request searching for Objects in property tree
//
//==================================================================================================
class FieldService final : public FieldAccess::AsyncService, public ServiceInterface
{
public:
    grpc::Status GetArrayValue( grpc::ServerContext*        context,
                                const FieldRequest*         request,
                                GenericArray*               reply,
                                StateHandler<FieldRequest>* stateHandler );

    grpc::Status GetValue( grpc::ServerContext* context, const FieldRequest* request, GetterReply* reply );

    grpc::Status SetArrayValue( grpc::ServerContext*        context,
                                const GenericArray*         chunk,
                                SetterArrayReply*           reply,
                                StateHandler<GenericArray>* stateHandler );

    grpc::Status SetValue( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply );
    std::vector<AbstractCallback*> createCallbacks() override;
};

} // namespace caffa::rpc
