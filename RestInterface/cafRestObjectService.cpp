
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
#include "cafUuidGenerator.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa;
using namespace caffa::rpc;

using namespace std::placeholders;

RestObjectService::RestObjectService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "objects" );

    auto skeletonParameter = std::make_unique<RestTypedParameter<bool>>( "skeleton",
                                                                         RestParameter::Location::QUERY,
                                                                         false,
                                                                         "Whether to only send the skeleton" );
    skeletonParameter->setDefaultValue( false );

    auto uuidEntry = std::make_unique<RestPathEntry>( "{uuid}" );
    uuidEntry->setPathArgumentMatcher( &caffa::UuidGenerator::isUuid );
    auto uuidParameter = std::make_unique<RestTypedParameter<std::string>>( "uuid",
                                                                            RestParameter::Location::PATH,
                                                                            true,
                                                                            "The UUID of the object" );

    auto getAction =
        std::make_unique<RestAction>( http::verb::get, "Get object by UUID", "getObject", &RestObjectService::object );

    getAction->addResponse( http::status::ok,
                            std::make_unique<RestResponse>( anyObjectResponseContent(), "Specific object" ) );

    getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
    getAction->addParameter( uuidParameter->clone() );
    getAction->addParameter( skeletonParameter->clone() );
    getAction->setRequiresAuthentication( false );
    getAction->setRequiresSession( true );

    uuidEntry->addAction( std::move( getAction ) );

    auto fieldEntry        = std::make_unique<RestPathEntry>( "fields" );
    auto fieldKeywordEntry = std::make_unique<RestPathEntry>( "{fieldKeyword}" );
    fieldKeywordEntry->setPathArgumentMatcher( &caffa::ObjectHandle::isValidKeyword );

    auto fieldKeywordParameter = std::make_unique<RestTypedParameter<std::string>>( "fieldKeyword",
                                                                                    RestParameter::Location::PATH,
                                                                                    true,
                                                                                    "The keyword of the field" );

    auto indexParameter = std::make_unique<RestTypedParameter<int>>( "index",
                                                                     RestParameter::Location::QUERY,
                                                                     false,
                                                                     "The index of the field (for array fields)" );

    {
        auto fieldGetAction = std::make_unique<RestAction>( http::verb::get,
                                                            "Get field value",
                                                            "getFieldValue",
                                                            &RestObjectService::performFieldOperation );
        fieldGetAction->addResponse( http::status::ok,
                                     std::make_unique<RestResponse>( anyFieldResponseContent(), "Specific field" ) );
        fieldGetAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldGetAction->addParameter( fieldKeywordParameter->clone() );
        fieldGetAction->addParameter( indexParameter->clone() );
        fieldGetAction->addParameter( skeletonParameter->clone() );
        fieldGetAction->setRequiresAuthentication( false );
        fieldGetAction->setRequiresSession( true );
        fieldKeywordEntry->addAction( std::move( fieldGetAction ) );
    }
    {
        auto fieldPutAction = std::make_unique<RestAction>( http::verb::put,
                                                            "Set field value",
                                                            "replaceFieldValue",
                                                            &RestObjectService::performFieldOperation );
        fieldPutAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        fieldPutAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldPutAction->addParameter( fieldKeywordParameter->clone() );
        fieldPutAction->addParameter( indexParameter->clone() );
        fieldPutAction->setRequiresAuthentication( false );
        fieldPutAction->setRequiresSession( true );

        auto requestBody       = nlohmann::json::object();
        requestBody["content"] = anyFieldResponseContent();

        fieldPutAction->setRequestBodySchema( requestBody );
        fieldKeywordEntry->addAction( std::move( fieldPutAction ) );
    }
    {
        auto fieldPostAction = std::make_unique<RestAction>( http::verb::post,
                                                             "Insert into field",
                                                             "insertFieldValue",
                                                             &RestObjectService::performFieldOperation );
        fieldPostAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        fieldPostAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldPostAction->addParameter( fieldKeywordParameter->clone() );
        fieldPostAction->addParameter( indexParameter->clone() );
        fieldPostAction->setRequiresAuthentication( false );
        fieldPostAction->setRequiresSession( true );

        auto requestBody       = nlohmann::json::object();
        requestBody["content"] = anyFieldResponseContent();

        fieldPostAction->setRequestBodySchema( requestBody );
        fieldKeywordEntry->addAction( std::move( fieldPostAction ) );
    }
    {
        auto fieldDeleteAction = std::make_unique<RestAction>( http::verb::delete_,
                                                               "Delete from field",
                                                               "deleteFieldValue",
                                                               &RestObjectService::performFieldOperation );
        fieldDeleteAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        fieldDeleteAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldDeleteAction->addParameter( fieldKeywordParameter->clone() );
        fieldDeleteAction->addParameter( indexParameter->clone() );
        fieldDeleteAction->setRequiresAuthentication( false );
        fieldDeleteAction->setRequiresSession( true );

        fieldKeywordEntry->addAction( std::move( fieldDeleteAction ) );
    }
    fieldEntry->addEntry( std::move( fieldKeywordEntry ) );

    auto methodEntry        = std::make_unique<RestPathEntry>( "methods" );
    auto methodKeywordEntry = std::make_unique<RestPathEntry>( "{methodKeyword}" );
    methodKeywordEntry->setPathArgumentMatcher( &caffa::ObjectHandle::isValidKeyword );
    auto methodKeywordParameter = std::make_unique<RestTypedParameter<std::string>>( "methodKeyword",
                                                                                     RestParameter::Location::PATH,
                                                                                     true,
                                                                                     "The keyword of the method" );

    {
        auto methodExecuteAction = std::make_unique<RestAction>( http::verb::post,
                                                                 "Execute method",
                                                                 "executeMethod",
                                                                 &RestObjectService::executeMethod );

        methodExecuteAction->addResponse( http::status::ok,
                                          std::make_unique<RestResponse>( anyFieldResponseContent(), "Success" ) );
        methodExecuteAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        methodExecuteAction->addParameter( methodKeywordParameter->clone() );
        methodExecuteAction->setRequiresAuthentication( false );
        methodExecuteAction->setRequiresSession( true );

        auto methodContent = nlohmann::json{ { "application/json", { { "schema", nlohmann::json::object() } } } };

        auto methodBody = nlohmann::json{ { "description", "JSON content representing the parameters of the method" },
                                          { "content", methodContent } };

        methodExecuteAction->setRequestBodySchema( methodBody );
        methodKeywordEntry->addAction( std::move( methodExecuteAction ) );
        methodEntry->addEntry( std::move( methodKeywordEntry ) );
    }

    uuidEntry->addEntry( std::move( fieldEntry ) );
    uuidEntry->addEntry( std::move( methodEntry ) );

    m_requestPathRoot->addEntry( std::move( uuidEntry ) );
}

nlohmann::json RestObjectService::anyObjectResponseContent()
{
    auto objectContent = nlohmann::json::object();
    auto classArray    = nlohmann::json::array();
    for ( auto classKeyword : DefaultObjectFactory::instance()->classes() )
    {
        auto schemaRef = nlohmann::json{ { "$ref", "#/components/object_schemas/" + classKeyword } };
        classArray.push_back( schemaRef );
    }
    auto classSchema                  = nlohmann::json{ { "oneOf", classArray }, { "discriminator", "keyword" } };
    objectContent["application/json"] = { { "schema", classSchema } };
    return objectContent;
}

nlohmann::json RestObjectService::anyFieldResponseContent()
{
    auto objectContent = nlohmann::json::object();
    auto classArray    = nlohmann::json::array();
    for ( auto classKeyword : DefaultObjectFactory::instance()->classes() )
    {
        auto schemaRef = nlohmann::json{ { "$ref", "#/components/object_schemas/" + classKeyword } };
        classArray.push_back( schemaRef );
    }

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
    return nlohmann::json{ { "application/json", { { "schema", { { "oneOf", oneOf } } } } } };
}

RestObjectService::ServiceResponse RestObjectService::perform( http::verb             verb,
                                                               std::list<std::string> path,
                                                               const nlohmann::json&  queryParams,
                                                               const nlohmann::json&  body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_tuple( http::status::bad_request, "Path not found", nullptr );
    }
    CAFFA_DEBUG( "Found request handler: " << request->name() );

    return request->perform( verb, pathArguments, queryParams, body );
}

//--------------------------------------------------------------------------------------------------
/// The object service uses session uuids to decide if it accepts the request or not
//--------------------------------------------------------------------------------------------------
bool RestObjectService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }
    return request->requiresAuthentication( verb );
}

bool RestObjectService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }
    return request->requiresSession( verb );
}

std::map<std::string, nlohmann::json> RestObjectService::servicePathEntries() const
{
    CAFFA_DEBUG( "Get service path entries" );

    auto services = nlohmann::json::object();

    RequestFinder finder( m_requestPathRoot.get() );
    finder.search();

    CAFFA_DEBUG( "Got " << finder.allPathEntriesWithActions().size() << " service path entries" );

    for ( const auto& [path, request] : finder.allPathEntriesWithActions() )
    {
        CAFFA_DEBUG( "Got path: " << path );
        services[path] = request->schema();
    }
    return services;
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

std::pair<caffa::ObjectHandle*, RestObjectService::ServiceResponse>
    RestObjectService::findObject( std::list<std::string>& pathArguments, const nlohmann::json& queryParams )
{
    caffa::SessionMaintainer session;

    if ( pathArguments.empty() )
    {
        return std::make_pair( nullptr, std::make_tuple( http::status::bad_request, "Object uuid not specified", nullptr ) );
    }

    auto uuid = pathArguments.front();
    pathArguments.pop_front();

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    if ( !session || session->isExpired() )
    {
        return std::make_pair( nullptr, std::make_tuple( http::status::forbidden, "No valid session provided", nullptr ) );
    }

    CAFFA_TRACE( "Trying to look for uuid '" << uuid << "'" );

    auto object = findCafObjectFromUuid( session.get(), uuid );
    if ( !object )
    {
        return std::make_pair( nullptr,
                               std::make_tuple( http::status::not_found, "Object " + uuid + " not found", nullptr ) );
    }

    return std::make_pair( object, std::make_tuple( http::status::ok, "", nullptr ) );
}

RestObjectService::ServiceResponse RestObjectService::performFieldOperation( http::verb verb,
                                                                             const std::list<std::string>& pathArguments,
                                                                             const nlohmann::json& queryParams,
                                                                             const nlohmann::json& body )
{
    CAFFA_DEBUG( "Full arguments for field operation: "
                 << caffa::StringTools::join( pathArguments.begin(), pathArguments.end(), "/" ) );

    auto arguments          = pathArguments;
    auto [object, response] = findObject( arguments, queryParams );
    if ( !object )
    {
        return response;
    }

    if ( arguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "field keyword not provided", nullptr );
    }

    CAFFA_DEBUG( "Got arguments:" );
    for ( auto arg : arguments )
    {
        CAFFA_DEBUG( "  /" << arg );
    }

    auto keyword = arguments.front();

    if ( auto field = object->findField( keyword ); field )
    {
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
    RestObjectService::replaceFieldValue( FieldHandle* field, int64_t index, const nlohmann::json& body )
{
    auto scriptability = field->capability<FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field " + field->keyword() + " is not remote writable", nullptr );

    auto ioCapability = field->capability<FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    JsonSerializer serializer;

    try
    {
        auto childField = dynamic_cast<ChildFieldHandle*>( field );
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

        auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
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
    RestObjectService::insertFieldValue( FieldHandle* field, int64_t index, const nlohmann::json& body )
{
    auto scriptability = field->capability<FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field " + field->keyword() + " is not remote writable", nullptr );

    auto ioCapability = field->capability<FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    JsonSerializer serializer;

    try
    {
        auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
        if ( childArrayField )
        {
            CAFFA_DEBUG( "Inserting into child array field with index " << index );
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

RestObjectService::ServiceResponse RestObjectService::deleteFieldValue( FieldHandle* field, int64_t index )
{
    auto scriptability = field->capability<FieldScriptingCapability>();
    if ( !scriptability || !scriptability->isWritable() )
        return std::make_tuple( http::status::forbidden, "Field is not remote writable", nullptr );

    auto ioCapability = field->capability<FieldJsonCapability>();
    if ( !ioCapability )
    {
        return std::make_tuple( http::status::forbidden,
                                "Field " + field->keyword() + " found, but it has no JSON capability",
                                nullptr );
    }

    JsonSerializer serializer;

    try
    {
        auto childField = dynamic_cast<ChildFieldHandle*>( field );
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

        auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field );
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

RestObjectService::ServiceResponse RestObjectService::object( http::verb                    verb,
                                                              const std::list<std::string>& pathArguments,
                                                              const nlohmann::json&         queryParams,
                                                              const nlohmann::json&         body )
{
    auto arguments          = pathArguments;
    auto [object, response] = findObject( arguments, queryParams );
    if ( !object )
    {
        return response;
    }

    bool           skeleton = queryParams.contains( "skeleton" ) && queryParams["skeleton"].get<bool>();
    nlohmann::json jsonObject;
    if ( skeleton )
    {
        jsonObject = createJsonSkeletonFromProjectObject( object );
    }
    else
    {
        jsonObject = createJsonFromProjectObject( object );
    }
    return std::make_tuple( http::status::ok, jsonObject.dump(), nullptr );
}

RestObjectService::ServiceResponse RestObjectService::executeMethod( http::verb                    verb,
                                                                     const std::list<std::string>& pathArguments,
                                                                     const nlohmann::json&         queryParams,
                                                                     const nlohmann::json&         body )
{
    caffa::SessionMaintainer session;
    auto                     arguments = pathArguments;

    if ( arguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "Object uuid not specified", nullptr );
    }

    auto uuid = arguments.front();
    arguments.pop_front();

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    if ( !session || session->isExpired() )
    {
        return std::make_tuple( http::status::forbidden, "No valid session provided", nullptr );
    }

    CAFFA_TRACE( "Trying to look for uuid '" << uuid << "'" );

    auto object = findCafObjectFromUuid( session.get(), uuid );
    if ( !object )
    {
        return std::make_tuple( http::status::not_found, "Object " + uuid + " not found", nullptr );
    }

    if ( arguments.empty() )
    {
        return std::make_tuple( http::status::bad_request, "method keyword not provided", nullptr );
    }

    auto keyword = arguments.front();

    if ( auto method = object->findMethod( keyword ); method )
    {
        CAFFA_TRACE( "Found method: " << method->keyword() );

        auto result = method->execute( *session, body.dump() );
        return std::make_tuple( http::status::ok, result, nullptr );
    }

    return std::make_tuple( http::status::not_found, "No method named " + keyword + " found", nullptr );
}