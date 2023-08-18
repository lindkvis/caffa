
// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2023- Kontur AS
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
#include "cafRestObjectService.h"

#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcServer.h"
#include "cafSession.h"

#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafRestServerApplication.h"
#include "cafRpcObjectConversion.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa::rpc;

RestObjectService::ServiceResponse RestObjectService::perform( http::verb                    verb,
                                                               const std::list<std::string>& path,
                                                               const nlohmann::json&         arguments,
                                                               const nlohmann::json&         metaData )
{
    std::string session_uuid = "";
    if ( arguments.contains( "session_uuid" ) )
    {
        session_uuid = arguments["session_uuid"].get<std::string>();
    }
    else if ( metaData.contains( "session_uuid" ) )
    {
        session_uuid = metaData["session_uuid"].get<std::string>();
    }

    if ( session_uuid.empty() )
    {
        CAFFA_WARNING( "No session uuid provided" );
    }
    auto session = RestServerApplication::instance()->getExistingSession( session_uuid );

    if ( !session && RestServerApplication::instance()->requiresValidSession() )
    {
        return std::make_tuple( http::status::unauthorized, "No session provided", nullptr );
    }

    bool skeleton = metaData.contains( "skeleton" ) && metaData["skeleton"].get<bool>();
    bool replace  = metaData.contains( "replace" ) && metaData["replace"].get<bool>();

    if ( path.empty() )
    {
        return documents( session.get(), skeleton );
    }
    else
    {
        caffa::ObjectHandle* object;

        auto documentId  = path.front();
        auto reducedPath = path;
        reducedPath.pop_front();

        CAFFA_TRACE( "Trying to look for document id '" << documentId << "'" );
        if ( documentId == "uuid" && !reducedPath.empty() )
        {
            auto uuid = reducedPath.front();
            CAFFA_TRACE( "Using uuid: " << uuid );
            object = findObject( reducedPath.front(), session.get() );
            reducedPath.pop_front();
            if ( !object )
            {
                return std::make_tuple( http::status::not_found, "Object " + uuid + " not found", nullptr );
            }
        }
        else
        {
            object = document( documentId, session.get() );
            if ( !object )
            {
                return std::make_tuple( http::status::not_found, "Document not found '" + documentId + "'", nullptr );
            }
        }

        CAFFA_ASSERT( object );
        if ( reducedPath.empty() )
        {
            if ( skeleton )
            {
                return std::make_tuple( http::status::ok, createJsonSkeletonFromProjectObject( object ).dump(), nullptr );
            }
            else
            {
                return std::make_tuple( http::status::ok, createJsonFromProjectObject( object ).dump(), nullptr );
            }
        }
        return perform( verb, object, reducedPath, arguments, skeleton, replace );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Document* RestObjectService::document( const std::string& documentId, const caffa::Session* session )
{
    CAFFA_TRACE( "Got document request for " << documentId );

    auto document = RestServerApplication::instance()->document( documentId, session );
    if ( document )
    {
        CAFFA_TRACE( "Found document with UUID: " << document->uuid() );
        return document.get();
    }
    return nullptr;
}

caffa::ObjectHandle* RestObjectService::findObject( const std::string& uuid, const caffa::Session* session )
{
    return findCafObjectFromUuid( session, uuid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestObjectService::ServiceResponse RestObjectService::documents( const caffa::Session* session, bool skeleton )
{
    CAFFA_DEBUG( "Got list document request for" );

    auto documents = RestServerApplication::instance()->documents( session );
    CAFFA_DEBUG( "Found " << documents.size() << " document" );

    auto jsonResult = nlohmann::json::array();
    for ( auto document : documents )
    {
        if ( skeleton )
        {
            jsonResult.push_back( createJsonSkeletonFromProjectObject( document.get() ) );
        }
        else
        {
            jsonResult.push_back( createJsonFromProjectObject( document.get() ) );
        }
    }
    return std::make_tuple( http::status::ok, jsonResult.dump(), nullptr );
}

RestObjectService::ServiceResponse RestObjectService::perform( http::verb                    verb,
                                                               caffa::ObjectHandle*          object,
                                                               const std::list<std::string>& path,
                                                               const nlohmann::json&         arguments,
                                                               bool                          skeleton,
                                                               bool                          replace )
{
    auto [fieldOrMethod, index] = findFieldOrMethod( object, path );

    if ( !fieldOrMethod )
    {
        return std::make_tuple( http::status::not_found, "Failed to find field or method", nullptr );
    }

    auto field = dynamic_cast<caffa::FieldHandle*>( fieldOrMethod );
    if ( field )
    {
        if ( verb == http::verb::get )
        {
            return getFieldValue( field, index, skeleton );
        }
        else if ( verb == http::verb::put )
        {
            return putFieldValue( field, index, arguments, replace );
        }
        else if ( verb == http::verb::delete_ )
        {
            return deleteChildObject( field, index, arguments );
        }
        return std::make_tuple( http::status::bad_request, "Verb not implemented", nullptr );
    }

    auto method = dynamic_cast<caffa::MethodHandle*>( fieldOrMethod );
    CAFFA_ASSERT( method );

    CAFFA_DEBUG( "Found method: " << method->keyword() );

    auto result = method->execute( arguments.dump() );
    return std::make_tuple( http::status::ok, result, nullptr );
}

std::pair<caffa::ObjectAttribute*, int64_t> RestObjectService::findFieldOrMethod( caffa::ObjectHandle*          object,
                                                                                  const std::list<std::string>& path )
{
    auto pathComponent = path.front();

    std::regex  arrayRgx( "(.+)\\[(\\d+)\\]" );
    std::smatch matches;

    std::string fieldOrMethodName = pathComponent;
    int64_t     index             = -1;
    if ( std::regex_match( pathComponent, matches, arrayRgx ) )
    {
        if ( matches.size() == 3 )
        {
            fieldOrMethodName = matches[1];
            auto optindex     = caffa::StringTools::toInt64( matches[2] );
            if ( optindex ) index = *optindex;
        }
    }

    CAFFA_TRACE( "Looking for field '" << fieldOrMethodName << "', index: " << index );
    if ( auto currentLevelField = object->findField( fieldOrMethodName ); currentLevelField )
    {
        auto reducedPath = path;
        reducedPath.pop_front();
        if ( !reducedPath.empty() )
        {
            auto childField = dynamic_cast<caffa::ChildFieldBaseHandle*>( currentLevelField );
            if ( childField )
            {
                auto childObjects = childField->childObjects();
                if ( index == -1 )
                {
                    index = 0;
                }

                CAFFA_TRACE( "Looking for index " << index << " in an array of size " << childObjects.size() );
                if ( index >= static_cast<int64_t>( childObjects.size() ) )
                {
                    return std::make_pair( nullptr, -1 );
                }
                return findFieldOrMethod( childObjects[index].get(), reducedPath );
            }
            return std::make_pair( nullptr, -1 );
        }
        return std::make_pair( currentLevelField, index );
    }
    else if ( auto currentLevelMethod = object->findMethod( fieldOrMethodName ); currentLevelMethod )
    {
        return std::make_pair( currentLevelMethod, -1 );
    }
    return std::make_pair( nullptr, -1 );
}

RestObjectService::ServiceResponse
    RestObjectService::getFieldValue( const caffa::FieldHandle* field, int64_t index, bool skeleton )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isReadable() )
        return std::make_tuple( http::status::forbidden, "Field is not remote readable", nullptr );

    auto ioCapability = field->capability<caffa::FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }
    auto childField = dynamic_cast<const caffa::ChildFieldBaseHandle*>( field );
    try
    {
        JsonSerializer serializer;
        if ( childField )
        {
            // The skeleton serialization only makes sense for objects
            if ( skeleton ) serializer.setSerializationType( Serializer::SerializationType::DATA_SKELETON );

            if ( index >= 0 )
            {
                auto childObjects = childField->childObjects();
                if ( index >= static_cast<int64_t>( childObjects.size() ) )
                {
                    CAFFA_ERROR( "Failed to get field value for  '" << field->keyword()
                                                                    << "' because index was out of range" );
                    return std::make_tuple( http::status::forbidden, "index out of range", nullptr );
                }
                return std::make_tuple( http::status::ok,
                                        serializer.writeObjectToString( childObjects[index].get() ),
                                        nullptr );
            }
            nlohmann::json jsonValue;
            ioCapability->writeToJson( jsonValue, serializer );

            return std::make_tuple( http::status::ok, jsonValue.dump(), nullptr );
        }

        nlohmann::json jsonValue;
        ioCapability->writeToJson( jsonValue, serializer );

        return std::make_tuple( http::status::ok, jsonValue.dump(), nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to get field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}

RestObjectService::ServiceResponse
    RestObjectService::putFieldValue( caffa::FieldHandle* field, int64_t index, const nlohmann::json& arguments, bool replace )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field is not remote writable", nullptr );

    auto ioCapability = field->capability<caffa::FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    JsonSerializer serializer;

    try
    {
        auto childField = dynamic_cast<caffa::ChildFieldHandle*>( field );
        if ( childField )
        {
            if ( index >= 0 )
            {
                return std::make_tuple( http::status::bad_request,
                                        "Index does not make sense for a simple Child Field",
                                        nullptr );
            }
            ioCapability->readFromJson( arguments, serializer );
            return std::make_tuple( http::status::ok, "", nullptr );
        }

        auto childArrayField = dynamic_cast<caffa::ChildArrayFieldHandle*>( field );
        if ( childArrayField )
        {
            CAFFA_DEBUG( "Inserting into child array field with index " << index );
            if ( index >= 0 )
            {
                auto childObjects = childArrayField->childObjects();
                if ( index >= static_cast<int64_t>( childObjects.size() ) )
                {
                    auto object = serializer.createObjectFromString( arguments.dump() );
                    childArrayField->push_back_obj( object );
                    return std::make_tuple( http::status::ok, "", nullptr );
                }
                else if ( !replace )
                {
                    auto object = serializer.createObjectFromString( arguments.dump() );
                    childArrayField->insertAt( index, object );
                    return std::make_tuple( http::status::ok, "", nullptr );
                }
                else
                {
                    serializer.readObjectFromString( childObjects[index].get(), arguments.dump() );
                    return std::make_tuple( http::status::ok, "", nullptr );
                }
            }
        }
        ioCapability->readFromJson( arguments, serializer );
        return std::make_tuple( http::status::ok, "", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to set field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}

RestObjectService::ServiceResponse
    RestObjectService::deleteChildObject( caffa::FieldHandle* field, int64_t index, const nlohmann::json& arguments )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field is not remote writable", nullptr );

    auto ioCapability = field->capability<caffa::FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    JsonSerializer serializer;

    try
    {
        auto childField = dynamic_cast<caffa::ChildFieldHandle*>( field );
        if ( childField )
        {
            if ( index >= 0 )
            {
                return std::make_tuple( http::status::bad_request,
                                        "It does not make sense to provide an index for deleting objects from a single "
                                        "Child "
                                        "Field",
                                        nullptr );
            }
            else
            {
                childField->clear();
                return std::make_tuple( http::status::ok, "", nullptr );
            }
        }

        auto childArrayField = dynamic_cast<caffa::ChildArrayFieldHandle*>( field );
        if ( childArrayField )
        {
            if ( index >= 0 )
            {
                childArrayField->erase( index );
            }
            else
            {
                childArrayField->clear();
            }
            return std::make_tuple( http::status::ok, "", nullptr );
        }
        return std::make_tuple( http::status::bad_request, "Can not delete from a non-child field", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to delete child object for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}
