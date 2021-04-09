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
#pragma once

#include "cafGrpcCallbacks.h"
#include "cafGrpcServiceInterface.h"
#include "cafVariant.h"

#include "FieldService.grpc.pb.h"
#include "FieldService.pb.h"

#include <string>
#include <vector>

namespace caf
{
class ChildArrayFieldHandle;
class ChildFieldHandle;
class FieldHandle;
class Object;
class ObjectFactory;
class ObjectHandle;
class ProxyFieldHandle;
class ValueField;
} // namespace caf

namespace caf::rpc
{
class GetterReply;
class FieldRequest;
class SetterChunk;
class SetterReply;

struct AbstractDataHolder
{
    virtual size_t valueCount() const                                                                               = 0;
    virtual size_t valueSizeOf() const                                                                              = 0;
    virtual void   reserveReplyStorage( GetterReply* reply, size_t numberOfDataUnits ) const                        = 0;
    virtual void   addPackageValuesToReply( GetterReply* reply, size_t startIndex, size_t numberOfDataUnits ) const = 0;

    virtual size_t getValuesFromChunk( size_t startIndex, const SetterChunk* chunk ) = 0;
    virtual void   applyValuesToField( caf::ValueField* field )                      = 0;
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
    grpc::Status assignReply( GetterReply* reply );
    size_t       streamedValueCount() const override;
    size_t       totalValueCount() const override;
    void         finish() override;

    StateHandler<FieldRequest>* emptyClone() const override;

protected:
    caf::Object*                        m_fieldOwner;
    caf::ValueField*                    m_field;
    std::unique_ptr<AbstractDataHolder> m_dataHolder;
    size_t                              m_currentDataIndex;
};

/**
 *
 * State handler for client to server streaming
 *
 */
class SetterStateHandler : public StateHandler<SetterChunk>
{
public:
    SetterStateHandler();

    grpc::Status init( const SetterChunk* chunk ) override;
    grpc::Status receiveRequest( const SetterChunk* chunk, SetterReply* reply );
    size_t       streamedValueCount() const override;
    size_t       totalValueCount() const override;
    void         finish() override;

    StateHandler<SetterChunk>* emptyClone() const override;

protected:
    caf::Object*                        m_fieldOwner;
    caf::ValueField*                    m_field;
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
    grpc::Status GetValue( grpc::ServerContext*        context,
                           const FieldRequest*         request,
                           GetterReply*                reply,
                           StateHandler<FieldRequest>* stateHandler );

    grpc::Status SetValue( grpc::ServerContext*       context,
                           const SetterChunk*         chunk,
                           SetterReply*               reply,
                           StateHandler<SetterChunk>* stateHandler );

    std::vector<AbstractCallback*> registerCallbacks() override;
};

} // namespace caf::rpc
