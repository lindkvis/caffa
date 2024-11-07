
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

#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafRestServerApplication.h"
#include "cafRpcObjectConversion.h"
#include "cafStringTools.h"
#include "cafUuidGenerator.h"

#include <iostream>
#include <regex>
#include <vector>

using namespace caffa;
using namespace caffa::rpc;

using namespace std::placeholders;

RestObjectService::RestObjectService()
{
    auto validKeywordLambda = []( const std::string& arg ) { return ObjectHandle::isValidKeyword( arg ); };

    m_requestPathRoot = std::make_unique<RestPathEntry>( "objects" );

    auto skeletonParameter = std::make_unique<RestTypedParameter<bool>>( "skeleton",
                                                                         RestParameter::Location::QUERY,
                                                                         false,
                                                                         "Whether to only send the skeleton" );
    skeletonParameter->setDefaultValue( false );

    auto uuidEntry = std::make_unique<RestPathEntry>( "{uuid}" );
    uuidEntry->setPathArgumentMatcher( &UuidGenerator::isUuid );
    const auto uuidParameter = std::make_unique<RestTypedParameter<std::string>>( "uuid",
                                                                                  RestParameter::Location::PATH,
                                                                                  true,
                                                                                  "The UUID of the object" );

    uuidEntry->addAction( createObjectGetAction( { uuidParameter.get(), skeletonParameter.get() } ) );

    auto fieldEntry        = std::make_unique<RestPathEntry>( "fields" );
    auto fieldKeywordEntry = std::make_unique<RestPathEntry>( "{keyword}" );
    fieldKeywordEntry->setPathArgumentMatcher( validKeywordLambda );

    const auto keywordParameter = std::make_unique<RestTypedParameter<std::string>>( "keyword",
                                                                                     RestParameter::Location::PATH,
                                                                                     true,
                                                                                     "The keyword of the field" );

    auto indexParameter = std::make_unique<RestTypedParameter<int>>( "index",
                                                                     RestParameter::Location::QUERY,
                                                                     false,
                                                                     "The index of the field (for array fields)" );
    {
        auto getAction =
            createFieldOrMethodAction( http::verb::get,
                                       "Get a field value",
                                       "getFieldValue",
                                       { keywordParameter.get(), indexParameter.get(), skeletonParameter.get() } );
        getAction->addResponse( http::status::ok,
                                std::make_unique<RestResponse>( anyFieldResponseContent(), "Specific field" ) );
        getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldKeywordEntry->addAction( std::move( getAction ) );
    }

    {
        auto putAction =
            createFieldOrMethodAction( http::verb::put,
                                       "Replace a field value",
                                       "replaceFieldValue",
                                       { keywordParameter.get(), indexParameter.get(), skeletonParameter.get() } );

        putAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        putAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldKeywordEntry->addAction( std::move( putAction ) );
    }

    {
        auto postAction =
            createFieldOrMethodAction( http::verb::post,
                                       "Insert a field value",
                                       "insertFieldValue",
                                       { keywordParameter.get(), indexParameter.get(), skeletonParameter.get() } );

        postAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        postAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );

        auto requestBody       = json::object();
        requestBody["content"] = anyFieldResponseContent();

        postAction->setRequestBodySchema( requestBody );

        fieldKeywordEntry->addAction( std::move( postAction ) );
    }

    {
        auto deleteAction =
            createFieldOrMethodAction( http::verb::delete_,
                                       "Delete a field value",
                                       "deleteFieldValue",
                                       { keywordParameter.get(), indexParameter.get(), skeletonParameter.get() } );

        deleteAction->addResponse( http::status::accepted, RestResponse::emptyResponse( "Success" ) );
        deleteAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        fieldKeywordEntry->addAction( std::move( deleteAction ) );
    }

    fieldEntry->addEntry( std::move( fieldKeywordEntry ) );

    auto methodEntry        = std::make_unique<RestPathEntry>( "methods" );
    auto methodKeywordEntry = std::make_unique<RestPathEntry>( "{keyword}" );
    methodKeywordEntry->setPathArgumentMatcher( validKeywordLambda );
    auto methodKeywordParameter = std::make_unique<RestTypedParameter<std::string>>( "keyword",
                                                                                     RestParameter::Location::PATH,
                                                                                     true,
                                                                                     "The keyword of the method" );

    {
        auto methodExecuteAction =
            createFieldOrMethodAction( http::verb::post, "Execute method", "executeMethod", { keywordParameter.get() } );

        methodExecuteAction->addResponse( http::status::ok,
                                          std::make_unique<RestResponse>( anyFieldResponseContent(), "Success" ) );
        methodExecuteAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );

        json::object methodContent = { { "application/json", { { "schema", json::object() } } } };

        json::object methodBody = { { "description", "JSON content representing the parameters of the method" },
                                    { "content", methodContent } };

        methodExecuteAction->setRequestBodySchema( methodBody );
        methodKeywordEntry->addAction( std::move( methodExecuteAction ) );
        methodEntry->addEntry( std::move( methodKeywordEntry ) );
    }

    uuidEntry->addEntry( std::move( fieldEntry ) );
    uuidEntry->addEntry( std::move( methodEntry ) );

    m_requestPathRoot->addEntry( std::move( uuidEntry ) );
}

json::object RestObjectService::anyObjectResponseContent()
{
    json::object objectContent;
    json::array  classArray;
    for ( const auto& classKeyword : DefaultObjectFactory::instance()->classes() )
    {
        json::object schemaRef = { { "$ref", "#/components/object_schemas/" + classKeyword } };
        classArray.push_back( schemaRef );
    }
    json::object classSchema          = { { "oneOf", classArray }, { "discriminator", "keyword" } };
    objectContent["application/json"] = { { "schema", classSchema } };
    return objectContent;
}

json::object RestObjectService::anyFieldResponseContent()
{
    auto objectContent = json::object();
    auto classArray    = json::array();
    for ( const auto& classKeyword : DefaultObjectFactory::instance()->classes() )
    {
        json::object schemaRef = { { "$ref", "#/components/object_schemas/" + classKeyword } };
        classArray.push_back( schemaRef );
    }

    auto oneOf = json::array();
    for ( const auto types = ClientPassByRefObjectFactory::instance()->supportedDataTypes(); const auto& dataType : types )
    {
        oneOf.push_back( json::parse( dataType ) );
    }
    for ( auto classEntry : classArray )
    {
        oneOf.push_back( classEntry );
        json::object array = { { "type", "array" }, { "items", classEntry } };
        oneOf.push_back( array );
    }
    return { { "application/json", { { "schema", { { "oneOf", oneOf } } } } } };
}

RestObjectService::ServiceResponse RestObjectService::perform( http::verb             verb,
                                                               std::list<std::string> path,
                                                               const json::object&    queryParams,
                                                               const json::value&     body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_pair( http::status::bad_request,
                               "Object Path not found: " + StringTools::join( path.begin(), path.end(), "/" ) );
    }
    return request->perform( verb, pathArguments, queryParams, body );
}

//--------------------------------------------------------------------------------------------------
/// The object service uses session uuids to decide if it accepts the request or not
//--------------------------------------------------------------------------------------------------
bool RestObjectService::requiresAuthentication( const http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }
    return request->requiresAuthentication( verb );
}

bool RestObjectService::requiresSession( const http::verb verb, const std::list<std::string>& path ) const
{
    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return true;
    }
    return request->requiresSession( verb );
}

std::map<std::string, json::object> RestObjectService::servicePathEntries() const
{
    CAFFA_DEBUG( "Get service path entries" );

    std::map<std::string, json::object> services;

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

std::map<std::string, json::object> RestObjectService::serviceComponentEntries() const
{
    auto factory = DefaultObjectFactory::instance();

    auto schemas = json::object();

    for ( const auto& className : factory->classes() )
    {
        auto object        = factory->create( className );
        schemas[className] = createJsonSchemaFromProjectObject( object.get() );
    }
    return { { "object_schemas", schemas } };
}

RestObjectService::ServiceResponse
    RestObjectService::performFieldOrMethodOperation( http::verb                    verb,
                                                      const std::list<std::string>& pathArguments,
                                                      const json::object&           queryParams,
                                                      const json::value&            body )
{
    CAFFA_TRACE(
        "Full arguments for field operation: " << StringTools::join( pathArguments.begin(), pathArguments.end(), "/" ) );

    auto session = findSession( queryParams );
    if ( !session || session->isExpired() )
    {
        return std::make_pair( http::status::forbidden, "No valid session provided" );
    }

    auto arguments = pathArguments;

    if ( pathArguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Object uuid not specified" );
    }

    auto uuid = arguments.front();
    arguments.pop_front();

    CAFFA_TRACE( "Trying to look for uuid '" << uuid << "'" );

    auto object = findCafObjectFromUuid( session.get(), uuid );
    if ( !object )
    {
        return std::make_pair( http::status::not_found, "Object " + uuid + " not found" );
    }

    if ( arguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "field keyword not provided" );
    }

    auto keyword = arguments.front();

    if ( const auto method = object->findMethod( keyword ); method )
    {
        auto result = method->execute( session, json::dump( body ) );
        return std::make_pair( http::status::ok, result );
    }

    if ( const auto field = object->findField( keyword ); field )
    {
        int index = -1;
        if ( const auto it = queryParams.find( "index" ); it != queryParams.end() )
        {
            index = json::from_json<int>( it->value() );
        }

        bool skeleton = false;
        if ( const auto it = queryParams.find( "skeleton" ); it != queryParams.end() )
        {
            skeleton = json::from_json<bool>( it->value() );
        }

        if ( verb == http::verb::get )
        {
            return getFieldValue( field, index, skeleton );
        }
        if ( verb == http::verb::put )
        {
            return replaceFieldValue( field, index, body );
        }
        if ( verb == http::verb::post )
        {
            return insertFieldValue( field, index, body );
        }
        if ( verb == http::verb::delete_ )
        {
            return deleteFieldValue( field, index );
        }
        return std::make_pair( http::status::bad_request, "Verb not implemented" );
    }

    return std::make_pair( http::status::not_found,
                           "No field named " + keyword + " found in object " + std::string( object->classKeyword() ) );
}

RestObjectService::ServiceResponse
    RestObjectService::getFieldValue( const FieldHandle* field, const int64_t index, bool skeleton )
{
    if ( const auto scriptability = field->capability<FieldScriptingCapability>();
         !scriptability || !scriptability->isReadable() )
        return std::make_pair( http::status::forbidden, "Field " + field->keyword() + " is not remote readable" );

    const auto ioCapability = field->capability<FieldIoCapability>();
    if ( !ioCapability )
    {
        return std::make_pair( http::status::forbidden,
                               "Field " + field->keyword() + " found, but it has no JSON capability" );
    }
    const auto childField = dynamic_cast<const ChildFieldBaseHandle*>( field );
    try
    {
        JsonSerializer serializer;
        if ( childField )
        {
            // The skeleton serialization only makes sense for objects
            if ( skeleton ) serializer.setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON );

            if ( index >= 0 )
            {
                auto childObjects = childField->childObjects();
                if ( index >= static_cast<int64_t>( childObjects.size() ) )
                {
                    CAFFA_ERROR( "Failed to get field value for  '" << field->keyword()
                                                                    << "' because index was out of range" );
                    return std::make_pair( http::status::forbidden, "index out of range" );
                }
                return std::make_pair( http::status::ok, serializer.writeObjectToString( childObjects[index].get() ) );
            }
            json::value jsonValue;
            ioCapability->writeToJson( jsonValue, serializer );

            return std::make_pair( http::status::ok, json::dump( jsonValue ) );
        }

        json::value jsonValue;
        ioCapability->writeToJson( jsonValue, serializer );

        return std::make_pair( http::status::ok, json::dump( jsonValue ) );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to get field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_pair( http::status::internal_server_error, e.what() );
    }
}

RestObjectService::ServiceResponse
    RestObjectService::replaceFieldValue( FieldHandle* field, const int64_t index, const json::value& body )
{
    if ( const auto scriptability = field->capability<FieldScriptingCapability>();
         !scriptability || !scriptability->isWritable() )
        return std::make_pair( http::status::forbidden, "Field " + field->keyword() + " is not remote writable" );

    const auto ioCapability = field->capability<FieldIoCapability>();
    if ( !ioCapability )
    {
        return std::make_pair( http::status::forbidden,
                               "Field " + field->keyword() + " found, but it has no JSON capability" );
    }

    try
    {
        JsonSerializer serializer;

        if ( const auto childField = dynamic_cast<ChildFieldHandle*>( field ); childField )
        {
            if ( index >= 0 )
            {
                return std::make_pair( http::status::bad_request, "Index does not make sense for a simple Child Field" );
            }
            ioCapability->readFromJson( body, serializer );
            return std::make_pair( http::status::accepted, "" );
        }

        if ( const auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field ); childArrayField )
        {
            CAFFA_DEBUG( "Replacing child array object at index " << index );

            if ( const auto childObjects = childArrayField->childObjects();
                 index >= 0 && static_cast<size_t>( index ) < childObjects.size() )
            {
                serializer.readObjectFromString( childObjects[index].get(), json::dump( body ) );
                return std::make_pair( http::status::accepted, "" );
            }
            return std::make_pair( http::status::bad_request, "Index out of bounds for array field replace item request" );
        }
        ioCapability->readFromJson( body, serializer );
        return std::make_pair( http::status::accepted, "" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to set field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_pair( http::status::internal_server_error, e.what() );
    }
}

RestObjectService::ServiceResponse
    RestObjectService::insertFieldValue( FieldHandle* field, const int64_t index, const json::value& body )
{
    if ( const auto scriptability = field->capability<FieldScriptingCapability>();
         !scriptability || !scriptability->isWritable() )
        return std::make_pair( http::status::forbidden, "Field " + field->keyword() + " is not remote writable" );

    if ( const auto ioCapability = field->capability<FieldIoCapability>(); !ioCapability )
    {
        return std::make_pair( http::status::forbidden,
                               "Field " + field->keyword() + " found, but it has no JSON capability" );
    }

    try
    {
        if ( const auto childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field ); childArrayField )
        {
            CAFFA_DEBUG( "Inserting into child array field with index " << index );
            const auto object = JsonSerializer().createObjectFromJson( body.as_object() );

            if ( const auto existingSize = childArrayField->size();
                 index >= 0 && static_cast<size_t>( index ) < existingSize )
            {
                childArrayField->insertAt( index, object );
                return std::make_pair( http::status::accepted, "" );
            }

            childArrayField->push_back_obj( object );
            return std::make_pair( http::status::accepted, "" );
        }
        return std::make_pair( http::status::bad_request, "Insert only makes sense for a Child Array Fields" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to insert field value for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_pair( http::status::internal_server_error, e.what() );
    }
}

RestObjectService::ServiceResponse RestObjectService::deleteFieldValue( FieldHandle* field, const int64_t index )
{
    if ( const auto scriptability = field->capability<FieldScriptingCapability>();
         !scriptability || !scriptability->isWritable() )
        return std::make_pair( http::status::forbidden, "Field is not remote writable" );

    JsonSerializer serializer;

    try
    {
        if ( auto* childField = dynamic_cast<ChildFieldHandle*>( field ); childField )
        {
            if ( index >= 0 )
            {
                return std::make_pair( http::status::bad_request,
                                       "It does not make sense to provide an index for deleting objects from a single "
                                       "Child Field" );
            }
            childField->clear();
            return std::make_pair( http::status::accepted, "" );
        }

        if ( auto* childArrayField = dynamic_cast<ChildArrayFieldHandle*>( field ); childArrayField )
        {
            if ( index >= 0 )
            {
                childArrayField->erase( index );
            }
            else
            {
                childArrayField->clear();
            }
            return std::make_pair( http::status::accepted, "" );
        }
        return std::make_pair( http::status::bad_request, "Can not delete from a non-child field" );
    }
    catch ( const std::exception& e )
    {
        CAFFA_ERROR( "Failed to delete child object for  '" << field->keyword() << "' with error: '" << e.what() << "'" );
        return std::make_pair( http::status::internal_server_error, e.what() );
    }
}

RestObjectService::ServiceResponse RestObjectService::object( http::verb,
                                                              const std::list<std::string>& pathArguments,
                                                              const json::object&           queryParams,
                                                              const json::value& )
{
    auto session = findSession( queryParams );
    if ( !session || session->isExpired() )
    {
        return std::make_pair( http::status::forbidden, "No valid session provided" );
    }

    auto arguments = pathArguments;

    if ( arguments.empty() )
    {
        return std::make_pair( http::status::bad_request, "Object uuid not specified" );
    }

    const auto uuid = arguments.front();
    arguments.pop_front();

    CAFFA_TRACE( "Trying to look for uuid '" << uuid << "'" );

    const auto object = findCafObjectFromUuid( session.get(), uuid );
    if ( !object )
    {
        return std::make_pair( http::status::not_found, "Object " + uuid + " not found" );
    }

    CAFFA_TRACE( "Found object: " << object->classKeyword() << ", uuid: " << object->uuid() );

    json::object jsonObject;
    if ( const auto it = queryParams.find( "skeleton" ); it != queryParams.end() )
    {
        jsonObject = createJsonSkeletonFromProjectObject( object.get() );
    }
    else
    {
        jsonObject = createJsonFromProjectObject( object.get() );
    }
    return std::make_pair( http::status::ok, json::dump( jsonObject ) );
}

std::shared_ptr<Session> RestObjectService::findSession( const json::object& queryParams )
{
    std::shared_ptr<Session> session;

    if ( const auto it = queryParams.find( "session_uuid" ); it != queryParams.end() )
    {
        const auto session_uuid = json::from_json<std::string>( it->value() );
        session                 = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    return session;
}

std::unique_ptr<RestAction> RestObjectService::createObjectGetAction( const std::list<RestParameter*>& parameters )
{
    auto getAction =
        std::make_unique<RestAction>( http::verb::get, "Get object by UUID", "getObject", &RestObjectService::object );

    getAction->addResponse( http::status::ok,
                            std::make_unique<RestResponse>( anyObjectResponseContent(), "Specific object" ) );

    getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
    for ( const auto param : parameters )
    {
        getAction->addParameter( param->clone() );
    }
    getAction->setRequiresAuthentication( false );
    getAction->setRequiresSession( true );
    return getAction;
}

std::unique_ptr<RestAction> RestObjectService::createFieldOrMethodAction( http::verb                       verb,
                                                                          const std::string&               description,
                                                                          const std::string&               name,
                                                                          const std::list<RestParameter*>& parameters )
{
    auto fieldAction =
        std::make_unique<RestAction>( verb, description, name, &RestObjectService::performFieldOrMethodOperation );
    for ( const auto param : parameters )
    {
        fieldAction->addParameter( param->clone() );
    }
    fieldAction->setRequiresAuthentication( false );
    fieldAction->setRequiresSession( true );
    return fieldAction;
}
