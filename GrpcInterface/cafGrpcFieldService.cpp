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
static caffa::Field<std::vector<DataType>>* castToVectorField( caffa::FieldHandle* field )
{
    return dynamic_cast<caffa::Field<std::vector<DataType>>*>( field );
}

template <typename DataType>
static const caffa::Field<std::vector<DataType>>* castToVectorField( const caffa::FieldHandle* field )
{
    return dynamic_cast<const caffa::Field<std::vector<DataType>>*>( field );
}

template <typename DataType>
std::vector<DataType> createVector( const ::google::protobuf::RepeatedField<DataType>& repeatedField )
{
    return std::vector<DataType>( repeatedField.begin(), repeatedField.end() );
}

bool FieldService::addValuesToReply( const caffa::FieldHandle* field, GenericArray* array )
{
    if ( auto vectorField = castToVectorField<int>( field ); vectorField )
    {
        CAFFA_TRACE( "Got " << vectorField->value().size() << " ints to send" );
        auto values                                = vectorField->value();
        *( array->mutable_ints()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<uint32_t>( field ); vectorField )
    {
        auto values                                 = vectorField->value();
        *( array->mutable_uints()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<int64_t>( field ); vectorField )
    {
        auto values                                  = vectorField->value();
        *( array->mutable_int64s()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<uint64_t>( field ); vectorField )
    {
        auto values                                   = vectorField->value();
        *( array->mutable_uint64s()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<double>( field ); vectorField )
    {
        auto values                                   = vectorField->value();
        *( array->mutable_doubles()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<float>( field ); vectorField )
    {
        auto values                                  = vectorField->value();
        *( array->mutable_floats()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<std::string>( field ); vectorField )
    {
        auto values                                   = vectorField->value();
        *( array->mutable_strings()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto vectorField = castToVectorField<bool>( field ); vectorField )
    {
        auto values                                 = vectorField->value();
        *( array->mutable_bools()->mutable_data() ) = { values.begin(), values.end() };
        return true;
    }
    else if ( auto childField = dynamic_cast<const ChildFieldHandle*>( field ); childField )
    {
        auto children = field->childObjects();
        for ( auto child : children )
        {
            RpcObject* rpcObject = array->mutable_objects()->add_objects();
            ObjectService::copyProjectObjectFromCafToRpc( child, rpcObject );
            CAFFA_TRACE( "Got child field values" );
        }
        return true;
    }
    else if ( auto childField = dynamic_cast<const ChildArrayFieldHandle*>( field ); childField )
    {
        auto children = field->childObjects();
        for ( auto child : children )
        {
            RpcObject* rpcObject = array->mutable_objects()->add_objects();
            ObjectService::copyProjectObjectFromCafToRpc( child, rpcObject );
            CAFFA_TRACE( "Got child array field values" );
        }
        return true;
    }

    return false;
}

bool FieldService::applyValuesToField( const GenericArray* array, caffa::FieldHandle* field )
{
    if ( auto vectorField = castToVectorField<int>( field ); vectorField )
    {
        CAFFA_TRACE( "Setting " << array->ints().data().size() << " values" );
        vectorField->setValue( createVector( array->ints().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<uint32_t>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->uints().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<int64_t>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->int64s().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<uint64_t>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->uint64s().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<double>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->doubles().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<float>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->floats().data() ) );
        return true;
    }
    else if ( auto vectorField = castToVectorField<std::string>( field ); vectorField )
    {
        std::vector<std::string> strings( array->strings().data().begin(), array->strings().data().end() );
        vectorField->setValue( strings );
        return true;
    }
    else if ( auto vectorField = castToVectorField<bool>( field ); vectorField )
    {
        vectorField->setValue( createVector( array->bools().data() ) );
        return true;
    }
    else if ( auto childField = dynamic_cast<ChildFieldHandle*>( field ); childField )
    {
        CAFFA_ASSERT( false && "Not implemented" );
        return false;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::GetArrayValue( grpc::ServerContext* context, const FieldRequest* request, GenericArray* reply )
{
    CAFFA_TRACE( "Received GetArrayValue request" );

    CAFFA_ASSERT( request != nullptr );

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromFieldRequest( *request );
    CAFFA_ASSERT( fieldOwner );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && request->keyword() == scriptability->scriptFieldName() )
        {
            if ( addValuesToReply( field, reply ) )
            {
                return grpc::Status::OK;
            }
            else
            {
                return grpc::Status( grpc::UNIMPLEMENTED,
                                     "Data type not implemented for getting array fields of type " + request->keyword() );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found " + request->keyword() );
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

    auto [field, isScriptable] = fieldAndScriptableFromKeyword( fieldOwner, request->keyword() );

    if ( field )
    {
        if ( isScriptable )
        {
            bool isObjectField = dynamic_cast<caffa::ChildFieldHandle*>( field ) != nullptr;
            auto ioCapability  = field->capability<caffa::FieldIoCapability>();
            if ( ioCapability )
            {
                nlohmann::json jsonValue;
                JsonSerializer serializer;
                serializer.setSerializeDataValues( !isObjectField );
                ioCapability->writeToJson( jsonValue, serializer );
                reply->set_value( jsonValue.is_null() ? "" : jsonValue.dump() );
                CAFFA_TRACE( "Get " << fieldOwner->classKeyword() << " -> " << field->keyword() << " = "
                                    << reply->value() );

                return grpc::Status::OK;
            }
        }
        return grpc::Status( grpc::FAILED_PRECONDITION,
                             "Field " + request->keyword() +
                                 " found, but it either isn't scriptable or does not have I/O capability" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
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
                std::unique_ptr<caffa::ObjectHandle> newCafObject =
                    caffa::JsonSerializer( DefaultObjectFactory::instance() ).createObjectFromString( request->value() );
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
grpc::Status FieldService::SetArrayValue( grpc::ServerContext* context, const GenericArray* array, NullMessage* reply )
{
    CAFFA_ASSERT( array->has_request() );
    auto fieldRequest = array->request();
    auto fieldOwner   = ObjectService::findCafObjectFromFieldRequest( fieldRequest );

    for ( auto field : fieldOwner->fields() )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && scriptability->scriptFieldName() == fieldRequest.keyword() )
        {
            if ( applyValuesToField( array, field ) )
            {
                return grpc::Status::OK;
            }
            else
            {
                return grpc::Status( grpc::UNIMPLEMENTED, "Data type not implemented for setting array fields" );
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Object not found" );
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

    auto [field, isScriptable] = fieldAndScriptableFromKeyword( fieldOwner, fieldRequest.keyword() );
    CAFFA_TRACE( "Field: " << field << ", " << isScriptable );
    if ( field != nullptr )
    {
        if ( isScriptable )
        {
            CAFFA_TRACE( "Set " << fieldOwner->classKeyword() << " -> " << fieldRequest.keyword() << " = "
                                << request->value() << "" );
            auto ioCapability = field->capability<caffa::FieldIoCapability>();
            if ( ioCapability )
            {
                auto           jsonValue = nlohmann::json::parse( request->value() );
                JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
                ioCapability->readFromJson( jsonValue, serializer );
                return grpc::Status::OK;
            }
        }
        std::string errMsg = "Field " + fieldRequest.keyword() +
                             " found, but it either isn't scriptable or does not have I/O capability";
        CAFFA_ERROR( errMsg );
        return grpc::Status( grpc::FAILED_PRECONDITION, errMsg );
    }
    CAFFA_ERROR( "Field " << fieldRequest.keyword() << " not found in " << fieldOwner->classKeyword() );
    return grpc::Status( grpc::NOT_FOUND, "Field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<AbstractCallback*> FieldService::createCallbacks()
{
    typedef FieldService Self;
    return { new UnaryCallback<Self, FieldRequest, GenericArray>( this, &Self::GetArrayValue, &Self::RequestGetArrayValue ),
             new UnaryCallback<Self, GenericArray, NullMessage>( this, &Self::SetArrayValue, &Self::RequestSetArrayValue ),
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

std::pair<caffa::FieldHandle*, bool> FieldService::fieldAndScriptableFromKeyword( const caffa::ObjectHandle* fieldOwner,
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
        auto scriptability     = field->capability<caffa::FieldScriptingCapability>();
        bool fieldIsScriptable = scriptability && keyword == scriptability->scriptFieldName();
        return std::make_pair( field, fieldIsScriptable );
    }
    return std::make_pair( nullptr, false );
}

} // namespace caffa::rpc
