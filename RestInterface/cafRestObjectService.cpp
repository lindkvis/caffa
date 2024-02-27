
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
#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcObjectConversion.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa::rpc;

RestObjectService::ServiceResponse RestObjectService::perform( http::verb             verb,
                                                               std::list<std::string> path,
                                                               const nlohmann::json&  queryParams,
                                                               const nlohmann::json&  body )
{
    caffa::SessionMaintainer session;

    CAFFA_ASSERT( !path.empty() );

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    path.pop_front();

    if ( path.empty() )
    {
        return std::make_tuple( http::status::bad_request, "Object uuid not specified", nullptr );
    }

    auto uuid = path.front();
    path.pop_front();

    CAFFA_TRACE( "Trying to look for uuid '" << uuid << "'" );

    auto object = findObject( uuid, session.get() );
    if ( !object )
    {
        return std::make_tuple( http::status::not_found, "Object " + uuid + " not found", nullptr );
    }

    CAFFA_ASSERT( object );
    if ( path.empty() )
    {
        bool skeleton = queryParams.contains( "skeleton" ) && body["skeleton"].get<bool>();
        if ( skeleton )
        {
            return std::make_tuple( http::status::ok, createJsonSkeletonFromProjectObject( object ).dump(), nullptr );
        }
        else
        {
            return std::make_tuple( http::status::ok, createJsonFromProjectObject( object ).dump(), nullptr );
        }
    }

    auto fieldOrMethod = path.front();
    path.pop_front();

    if ( path.empty() )
    {
        return std::make_tuple( http::status::bad_request, "No field or method keyword specified", nullptr );
    }

    auto keyword = path.front();
    path.pop_front();

    if ( fieldOrMethod == "fields" )
    {
        return performFieldOperation( *session, verb, object, keyword, queryParams, body );
    }
    else if ( fieldOrMethod == "methods" )
    {
        return performMethodOperation( *session, verb, object, keyword, queryParams, body );
    }
    else
    {
        return std::make_tuple( http::status::bad_request,
                                "No such target " + fieldOrMethod + " available for Objects",
                                nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
/// The object service uses session uuids to decide if it accepts the request or not
//--------------------------------------------------------------------------------------------------
bool RestObjectService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestObjectService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return true;
}

std::map<std::string, nlohmann::json> RestObjectService::servicePathEntries() const
{
    auto emptyResponseContent = nlohmann::json{ { "description", "Success" } };

    auto acceptedOrFailureResponses = nlohmann::json{ { HTTP_ACCEPTED, emptyResponseContent },
                                                      { "default", RestServiceInterface::plainErrorResponse() } };

    auto uuidParameter = nlohmann::json{ { "name", "uuid" },
                                         { "in", "path" },
                                         { "required", true },
                                         { "description", "The object UUID of the object to get" },
                                         { "schema", { { "type", "string" } } } };

    auto objectContent = nlohmann::json::object();
    auto classArray    = nlohmann::json::array();
    for ( auto classKeyword : DefaultObjectFactory::instance()->classes() )
    {
        auto schemaRef = nlohmann::json{ { "$ref", "#/components/object_schemas/" + classKeyword } };
        classArray.push_back( schemaRef );
    }
    auto classSchema                  = nlohmann::json{ { "oneOf", classArray }, { "discriminator", "keyword" } };
    objectContent["application/json"] = { { "schema", classSchema } };
    auto objectResponse = nlohmann::json{ { "description", "Specific object" }, { "content", objectContent } };

    auto getResponses =
        nlohmann::json{ { HTTP_OK, objectResponse }, { "default", RestServiceInterface::plainErrorResponse() } };

    auto object = nlohmann::json::object();
    object["get"] =
        createOperation( "getObject", "Get a particular object", uuidParameter, getResponses, nullptr, { "objects" } );
    object["delete"] = createOperation( "deleteObject",
                                        "Destroy a particular object",
                                        uuidParameter,
                                        acceptedOrFailureResponses,
                                        nullptr,
                                        { "objects" } );

    auto field = nlohmann::json::object();
    {
        auto fieldKeywordParameter = nlohmann::json{ { "name", "fieldKeyword" },
                                                     { "in", "path" },
                                                     { "required", true },
                                                     { "description", "The field keyword" },
                                                     { "schema", { { "type", "string" } } } };

        auto indexParameter = nlohmann::json{ { "name", "index" },
                                              { "in", "query" },
                                              { "required", false },
                                              { "default", -1 },
                                              { "description", "The index of the child object field." },
                                              { "schema", { { "type", "integer" } } } };
        auto skeletonParameter =
            nlohmann::json{ { "name", "skeleton" },
                            { "in", "query" },
                            { "required", false },
                            { "default", false },
                            { "description", "Whether to only retrieve the structure and not field values" },
                            { "schema", { { "type", "boolean" } } } };

        auto oneOf = nlohmann::json::array();
        for ( auto dataType : caffa::rpc::ClientPassByRefObjectFactory::instance()->supportedDataTypes() )
        {
            oneOf.push_back( nlohmann::json::parse( dataType ) );
        }
        for ( auto classEntry : classArray )
        {
            oneOf.push_back( classEntry );
            auto array = nlohmann::json{ { "type", "array" }, { "items", classEntry } };
            oneOf.push_back( array );
        }

        auto fieldValue   = nlohmann::json{ { "application/json", { { "schema", { { "oneOf", oneOf } } } } } };
        auto fieldContent = nlohmann::json{ { "description", "JSON content representing a valid Caffa data type" },
                                            { "content", fieldValue } };

        auto fieldParameters = nlohmann::json::array( { uuidParameter, fieldKeywordParameter, indexParameter } );
        auto fieldResponses =
            nlohmann::json{ { HTTP_OK, fieldContent }, { "default", RestServiceInterface::plainErrorResponse() } };

        field["put"]    = createOperation( "replaceFieldValue",
                                        "Replace a particular field value",
                                        fieldParameters,
                                        acceptedOrFailureResponses,
                                        fieldContent,
                                           { "fields" } );
        field["post"]   = createOperation( "insertFieldValue",
                                         "Insert a particular field value",
                                         fieldParameters,
                                         acceptedOrFailureResponses,
                                         fieldContent,
                                           { "fields" } );
        field["delete"] = createOperation( "deleteFieldValue",
                                           "Delete a value from a field. For array fields.",
                                           fieldParameters,
                                           acceptedOrFailureResponses,
                                           nullptr,
                                           { "fields" } );

        fieldParameters.push_back( skeletonParameter );
        field["get"] = createOperation( "getFieldValue",
                                        "Get a particular field value",
                                        fieldParameters,
                                        fieldResponses,
                                        nullptr,
                                        { "fields" } );
    }

    auto method = nlohmann::json::object();
    {
        auto methodKeywordParameter = nlohmann::json{ { "name", "methodKeyword" },
                                                      { "in", "path" },
                                                      { "required", true },
                                                      { "description", "The method keyword" },
                                                      { "schema", { { "type", "string" } } } };

        auto methodBody = nlohmann::json{ { "application/json", { { "schema", nlohmann::json::object() } } } };

        auto jsonContentObject = nlohmann::json{ { "description", "JSON content representing a valid Caffa data type" },
                                                 { "content", methodBody } };

        auto methodParameters = nlohmann::json::array( { uuidParameter, methodKeywordParameter } );
        auto methodResponses =
            nlohmann::json{ { HTTP_OK, jsonContentObject }, { "default", RestServiceInterface::plainErrorResponse() } };

        method["post"] = createOperation( "executeMethod",
                                          "Execute an Object Method",
                                          methodParameters,
                                          methodResponses,
                                          jsonContentObject,
                                          { "methods" } );
    }

    return { { "/objects/{uuid}", object },
             { "/objects/{uuid}/fields/{fieldKeyword}", field },
             { "/objects/{uuid}/methods/{methodKeyword}", method } };
}

std::map<std::string, nlohmann::json> RestObjectService::serviceComponentEntries() const
{
    auto factory = DefaultObjectFactory::instance();

    auto schemas = nlohmann::json::object();

    for ( auto className : factory->classes() )
    {
        auto object        = factory->create( className );
        schemas[className] = createJsonSchemaFromProjectObject( object.get() );
    }
    return { { "object_schemas", schemas } };
}

caffa::ObjectHandle* RestObjectService::findObject( const std::string& uuid, const caffa::Session* session )
{
    return findCafObjectFromUuid( session, uuid );
}

RestObjectService::ServiceResponse RestObjectService::performFieldOperation( std::shared_ptr<caffa::Session> session,
                                                                             http::verb                      verb,
                                                                             caffa::ObjectHandle*            object,
                                                                             const std::string&              keyword,
                                                                             const nlohmann::json& queryParams,
                                                                             const nlohmann::json& body )
{
    if ( auto field = object->findField( keyword ); field )
    {
        CAFFA_DEBUG( "Found field: " << field->keyword() );

        int  index    = queryParams.contains( "index" ) ? queryParams["index"].get<int>() : -1;
        bool skeleton = queryParams.contains( "skeleton" ) && queryParams["skeleton"].get<bool>();

        if ( verb == http::verb::get )
        {
            return getFieldValue( field, index, skeleton );
        }
        else if ( verb == http::verb::put )
        {
            return replaceFieldValue( field, index, body );
        }
        else if ( verb == http::verb::post )
        {
            return insertFieldValue( field, index, body );
        }

        else if ( verb == http::verb::delete_ )
        {
            return deleteFieldValue( field, index );
        }
        return std::make_tuple( http::status::bad_request, "Verb not implemented", nullptr );
    }
    return std::make_tuple( http::status::not_found, "No field named " + keyword + " found", nullptr );
}

RestObjectService::ServiceResponse RestObjectService::performMethodOperation( std::shared_ptr<caffa::Session> session,
                                                                              http::verb                      verb,
                                                                              caffa::ObjectHandle*            object,
                                                                              const std::string&              keyword,
                                                                              const nlohmann::json& queryParams,
                                                                              const nlohmann::json& body )
{
    if ( auto method = object->findMethod( keyword ); method )
    {
        CAFFA_TRACE( "Found method: " << method->keyword() );

        auto result = method->execute( session, body.dump() );
        return std::make_tuple( http::status::ok, result, nullptr );
    }

    return std::make_tuple( http::status::not_found, "No method named " + keyword + " found", nullptr );
}

RestObjectService::ServiceResponse
    RestObjectService::getFieldValue( const caffa::FieldHandle* field, int64_t index, bool skeleton )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isReadable() )
        return std::make_tuple( http::status::forbidden, "Field " + field->keyword() + " is not remote readable", nullptr );

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
    RestObjectService::replaceFieldValue( caffa::FieldHandle* field, int64_t index, const nlohmann::json& body )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field " + field->keyword() + " is not remote writable", nullptr );

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
            ioCapability->readFromJson( body, serializer );
            return std::make_tuple( http::status::accepted, "", nullptr );
        }

        auto childArrayField = dynamic_cast<caffa::ChildArrayFieldHandle*>( field );
        if ( childArrayField )
        {
            CAFFA_DEBUG( "Replacing child array object at index " << index );

            auto childObjects = childArrayField->childObjects();
            if ( index >= 0 && static_cast<size_t>( index ) < childObjects.size() )
            {
                auto childObjects = childArrayField->childObjects();
                serializer.readObjectFromString( childObjects[index].get(), body.dump() );
                return std::make_tuple( http::status::accepted, "", nullptr );
            }
            return std::make_tuple( http::status::bad_request,
                                    "Index out of bounds for array field replace item request",
                                    nullptr );
        }
        ioCapability->readFromJson( body, serializer );
        return std::make_tuple( http::status::accepted, "", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to set field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}

RestObjectService::ServiceResponse
    RestObjectService::insertFieldValue( caffa::FieldHandle* field, int64_t index, const nlohmann::json& body )
{
    auto scriptability = field->capability<caffa::FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field " + field->keyword() + " is not remote writable", nullptr );

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
        auto childArrayField = dynamic_cast<caffa::ChildArrayFieldHandle*>( field );
        if ( childArrayField )
        {
            CAFFA_INFO( "Inserting into child array field with index " << index );
            auto existingSize = childArrayField->size();
            if ( index >= 0 && static_cast<size_t>( index ) < existingSize )
            {
                auto object = serializer.createObjectFromString( body.dump() );
                childArrayField->insertAt( index, object );
                return std::make_tuple( http::status::accepted, "", nullptr );
            }
            else
            {
                auto object = serializer.createObjectFromString( body.dump() );
                childArrayField->push_back_obj( object );
                return std::make_tuple( http::status::accepted, "", nullptr );
            }
        }
        else
        {
            return std::make_tuple( http::status::bad_request, "Insert only makes sense for a Child Array Fields", nullptr );
        }
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to insert field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}

RestObjectService::ServiceResponse RestObjectService::deleteFieldValue( caffa::FieldHandle* field, int64_t index )
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
                return std::make_tuple( http::status::accepted, "", nullptr );
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
            return std::make_tuple( http::status::accepted, "", nullptr );
        }
        return std::make_tuple( http::status::bad_request, "Can not delete from a non-child field", nullptr );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to delete child object for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_tuple( http::status::internal_server_error, e.what(), nullptr );
    }
}
