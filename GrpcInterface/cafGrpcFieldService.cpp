// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
#include "cafGrpcFieldService.h"
#include "cafGrpcCallbacks.h"

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::GetValue( grpc::ServerContext* context, const FieldRequest* request, GenericValue* reply )
{
    CAFFA_ASSERT( request != nullptr );
    CAFFA_TRACE( "GetValue for field: " << request->keyword() << ", " << request->class_keyword() << ", "
                                        << request->uuid() );

    auto session = ServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromScriptNameAndUuid( session.get(),
                                                                                        request->class_keyword(),
                                                                                        request->uuid() );
    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    auto field = scriptableFieldFromKeyword( fieldOwner, request->keyword() );

    if ( field )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability && scriptability->isReadable() )
        {
            bool isObjectField = dynamic_cast<caffa::ChildFieldHandle*>( field ) != nullptr;
            auto ioCapability  = field->capability<caffa::FieldJsonCapability>();
            if ( ioCapability )
            {
                try
                {
                    nlohmann::json jsonValue;
                    JsonSerializer serializer;
                    serializer.setSerializeDataValues( !isObjectField || request->copy_object_values() );
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
                    CAFFA_ERROR( "gRPC Field::GetValue for '" << request->keyword() << "' failed with error: '"
                                                              << e.what() << "'" );
                    return grpc::Status( grpc::FAILED_PRECONDITION, std::string( "GetValue failed with " ) + e.what() );
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
grpc::Status FieldService::SetValue( grpc::ServerContext* context, const SetterRequest* request, NullMessage* reply )
{
    auto fieldRequest = request->field();

    auto session = ServerApplication::instance()->getExistingSession( fieldRequest.session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + fieldRequest.session().uuid() + "' is not valid" );
    }

    if ( session->type() == Session::Type::OBSERVING )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Observing sessions are not valid for controlling operation" );
    }

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromScriptNameAndUuid( session.get(),
                                                                                        fieldRequest.class_keyword(),
                                                                                        fieldRequest.uuid() );

    CAFFA_TRACE( "Received Set Request for " << fieldRequest.class_keyword() << "::" << fieldRequest.keyword()
                                             << ", uuid: " << fieldRequest.uuid() );

    if ( !fieldOwner ) return grpc::Status( grpc::NOT_FOUND, "Object not found" );

    auto field = scriptableFieldFromKeyword( fieldOwner, fieldRequest.keyword() );

    if ( field )
    {
        auto scriptability = field->capability<caffa::FieldScriptingCapability>();
        if ( scriptability )
        {
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
                CAFFA_ERROR( "gRPC Field::SetValue for '" << fieldRequest.keyword() << "' failed with error: '"
                                                          << e.what() << "'" );
                return grpc::Status( grpc::FAILED_PRECONDITION, std::string( "SetValue Failed with error: " ) + e.what() );
            }
        }
        else
        {
            return grpc::Status( grpc::FAILED_PRECONDITION,
                                 "Field " + fieldRequest.keyword() + " found, but it isn't writable" );
        }
    }
    std::string errMsg = std::string( "Field " ) + fieldRequest.keyword() + " not found in " + fieldOwner->classKeyword();
    CAFFA_ERROR( errMsg );
    return grpc::Status( grpc::NOT_FOUND, errMsg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status FieldService::ClearChildObjects( grpc::ServerContext* context, const FieldRequest* request, NullMessage* reply )
{
    CAFFA_ASSERT( request != nullptr );

    auto session = ServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    if ( session->type() == Session::Type::OBSERVING )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Observing sessions are not valid for controlling operations" );
    }

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromScriptNameAndUuid( session.get(),
                                                                                        request->class_keyword(),
                                                                                        request->uuid() );
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
                CAFFA_ERROR( "gRPC Field::ClearChildObjects for '" << request->keyword() << "' failed with error: '"
                                                                   << e.what() << "'" );
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

    auto session = ServerApplication::instance()->getExistingSession( request->session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + request->session().uuid() + "' is not valid" );
    }

    if ( session->type() == Session::Type::OBSERVING )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Observing sessions are not valid for controlling operations" );
    }

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromScriptNameAndUuid( session.get(),
                                                                                        request->class_keyword(),
                                                                                        request->uuid() );
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
                    CAFFA_ERROR( "gRPC Field::RemoveChildObjects for '"
                                 << request->keyword() << "' failed with error: '" << e.what() << "'" );
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

    auto session = ServerApplication::instance()->getExistingSession( fieldRequest.session().uuid() );
    if ( !session )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Session '" + fieldRequest.session().uuid() + "' is not valid" );
    }

    if ( session->type() == Session::Type::OBSERVING )
    {
        return grpc::Status( grpc::UNAUTHENTICATED, "Observing sessions are not valid for controlling operations" );
    }

    auto fieldOwner = ObjectService::ObjectService::findCafObjectFromScriptNameAndUuid( session.get(),
                                                                                        fieldRequest.class_keyword(),
                                                                                        fieldRequest.uuid() );
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
                    CAFFA_ERROR( "gRPC Field::InsertChildObjects for '" << fieldRequest.keyword() << "' failed with "
                                                                        << e.what() );
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
std::vector<AbstractCallback*> FieldService::createCallbacks()
{
    typedef FieldService Self;
    return { new ServiceCallback<Self, FieldRequest, GenericValue>( this, &Self::GetValue, &Self::RequestGetValue ),
             new ServiceCallback<Self, SetterRequest, NullMessage>( this, &Self::SetValue, &Self::RequestSetValue ),
             new ServiceCallback<Self, FieldRequest, NullMessage>( this,
                                                                   &Self::ClearChildObjects,
                                                                   &Self::RequestClearChildObjects ),
             new ServiceCallback<Self, FieldRequest, NullMessage>( this,
                                                                   &Self::RemoveChildObject,
                                                                   &Self::RequestRemoveChildObject ),
             new ServiceCallback<Self, SetterRequest, NullMessage>( this,
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
