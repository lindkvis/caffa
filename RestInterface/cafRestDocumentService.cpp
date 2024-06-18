
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
#include "cafRestDocumentService.h"

#include "cafRpcClientPassByRefObjectFactory.h"
#include "cafRpcServer.h"
#include "cafSession.h"

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldProxyAccessor.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafMethod.h"
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafObjectPerformer.h"
#include "cafPortableDataType.h"
#include "cafRestServerApplication.h"
#include "cafRpcObjectConversion.h"
#include "cafStringTools.h"

#include <functional>
#include <iostream>
#include <regex>
#include <vector>

using namespace caffa;
using namespace caffa::rpc;

using namespace std::placeholders;

RestDocumentService::RestDocumentService()
{
    m_requestPathRoot = std::make_unique<RestPathEntry>( "documents" );

    CAFFA_DEBUG( "Get service path entries" );
    // Create a trial tree with an object for each entry
    auto documents = rpc::RestServerApplication::instance()->defaultDocuments();

    for ( auto document : documents )
    {
        auto getAction =
            std::make_unique<RestAction>( http::verb::get,
                                          "Get " + document->id() + " document",
                                          "getDocument",
                                          std::bind( &RestDocumentService::document, document->id(), _1, _2, _3, _4 ) );

        getAction->addResponse( http::status::ok,
                                RestResponse::objectResponse( "#/components/object_schemas/" + document->classKeyword(),
                                                              document->classDocumentation() ) );

        getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        getAction->setRequiresAuthentication( false );
        getAction->setRequiresSession( true );

        auto documentEntry = std::make_unique<RestPathEntry>( document->id() );
        documentEntry->addAction( std::move( getAction ) );

        m_requestPathRoot->addEntry( std::move( documentEntry ) );
    }

    auto getAllAction =
        std::make_unique<RestAction>( http::verb::get, "Get all documents", "getDocuments", &RestDocumentService::documents );

    getAllAction->addResponse( http::status::ok,
                               RestResponse::objectArrayResponse( "#/components/object_schemas/Document", "All documents" ) );
    getAllAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );

    getAllAction->setRequiresAuthentication( false );
    getAllAction->setRequiresSession( true );

    m_requestPathRoot->addAction( std::move( getAllAction ) );
}

RestDocumentService::ServiceResponse RestDocumentService::perform( http::verb             verb,
                                                                   std::list<std::string> path,
                                                                   const nlohmann::json&  queryParams,
                                                                   const nlohmann::json&  body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_tuple( http::status::bad_request,
                                "Document Path not found: " + caffa::StringTools::join( path.begin(), path.end(), "/" ),
                                nullptr );
    }

    return request->perform( verb, pathArguments, queryParams, body );
}

//--------------------------------------------------------------------------------------------------
/// The object service uses session uuids to decide if it accepts the request or not
//--------------------------------------------------------------------------------------------------
bool RestDocumentService::requiresAuthentication( http::verb verb, const std::list<std::string>& path ) const
{
    return false;
}

bool RestDocumentService::requiresSession( http::verb verb, const std::list<std::string>& path ) const
{
    return true;
}

class PathCreator : public Inspector
{
public:
    PathCreator()
        : m_pathStack( { "/documents" } )
    {
        m_serializer.setSerializationType( JsonSerializer::SerializationType::PATH );
    }

    const std::map<std::string, nlohmann::json>& pathSchemas() const { return m_pathSchemas; }

    void visitObject( const ObjectHandle* object ) override
    {
        if ( auto doc = dynamic_cast<const Document*>( object ); doc )
        {
            m_pathStack.push_back( doc->id() );
            auto schema = nlohmann::json::object();
            m_serializer.writeObjectToJson( object, schema );
            auto path           = StringTools::join( m_pathStack.begin(), m_pathStack.end(), "/" );
            m_pathSchemas[path] = schema;
        }
    }

    void visitField( const FieldHandle* field ) override
    {
        m_pathStack.push_back( field->keyword() );
        auto schema = nlohmann::json::object();

        if ( auto scriptability = field->capability<FieldScriptingCapability>(); scriptability )
        {
            auto jsonCapability = field->capability<FieldJsonCapability>();
            CAFFA_ASSERT( jsonCapability );
            if ( scriptability->isReadable() )
            {
                auto operationId = field->keyword();
                operationId[0]   = (char)std::toupper( operationId[0] );
                operationId      = field->ownerObject()->classKeyword() + ".get" + operationId;

                auto getOperation =
                    nlohmann::json{ { "summary", "Get " + field->keyword() }, { "operationId", operationId } };

                auto fieldContent                = nlohmann::json::object();
                fieldContent["application/json"] = { { "schema", jsonCapability->jsonType() } };

                auto fieldResponse =
                    nlohmann::json{ { "description", field->documentation() }, { "content", fieldContent } };

                getOperation["responses"] = fieldResponse;
                schema["get"]             = getOperation;
            }
            if ( scriptability->isWritable() )
            {
                auto operationId = field->keyword();
                operationId[0]   = (char)std::toupper( operationId[0] );
                operationId      = field->ownerObject()->classKeyword() + ".set" + operationId;

                auto setOperation =
                    nlohmann::json{ { "summary", "Set " + field->keyword() }, { "operationId", operationId } };

                auto fieldContent                = nlohmann::json::object();
                fieldContent["application/json"] = { { "schema", jsonCapability->jsonType() } };

                auto acceptedOrFailureResponses =
                    nlohmann::json{ { RestServiceInterface::HTTP_ACCEPTED,
                                      { { "description", "Success" } },
                                      { "default", RestServiceInterface::plainErrorResponse() } } };

                setOperation["responses"]   = acceptedOrFailureResponses;
                setOperation["requestBody"] = nlohmann::json{ { "content", fieldContent } };
                schema["set"]               = setOperation;
            }
        }

        auto path           = StringTools::join( m_pathStack.begin(), m_pathStack.end(), "/" );
        m_pathSchemas[path] = schema;
    }

    void leaveObject( const ObjectHandle* object ) override
    {
        if ( auto doc = dynamic_cast<const Document*>( object ); doc )
        {
            m_pathStack.pop_back();
        }
    }
    void leaveField( const FieldHandle* field ) override { m_pathStack.pop_back(); }

private:
    std::list<std::string> m_pathStack;
    JsonSerializer         m_serializer;

    std::map<std::string, nlohmann::json> m_pathSchemas;
};

std::map<std::string, nlohmann::json> RestDocumentService::servicePathEntries() const
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

std::map<std::string, nlohmann::json> RestDocumentService::serviceComponentEntries() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServiceInterface::ServiceResponse RestDocumentService::document( const std::string&            documentId,
                                                                     http::verb                    verb,
                                                                     const std::list<std::string>& pathArguments,
                                                                     const nlohmann::json&         queryParams,
                                                                     const nlohmann::json&         body )
{
    caffa::SessionMaintainer session;

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    if ( RestServerApplication::instance()->requiresValidSession() && ( !session || session->isExpired() ) )
    {
        return std::make_tuple( http::status::forbidden, "No valid session provided", nullptr );
    }

    CAFFA_TRACE( "Got document request for " << documentId );

    auto document = RestServerApplication::instance()->document( documentId, session.get() );
    if ( document )
    {
        CAFFA_TRACE( "Found document with UUID: " << document->uuid() );
        nlohmann::json jsonDocument = createJsonFromProjectObject( document.get() );

        return std::make_tuple( http::status::ok, jsonDocument.dump(), nullptr );
    }
    return std::make_tuple( http::status::not_found, "Document " + documentId + " not found", nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestDocumentService::ServiceResponse RestDocumentService::documents( http::verb                    verb,
                                                                     const std::list<std::string>& pathArguments,
                                                                     const nlohmann::json&         queryParams,
                                                                     const nlohmann::json&         body )
{
    CAFFA_DEBUG( "Got list document request" );

    caffa::SessionMaintainer session;

    if ( queryParams.contains( "session_uuid" ) )
    {
        auto session_uuid = queryParams["session_uuid"].get<std::string>();
        session           = RestServerApplication::instance()->getExistingSession( session_uuid );
    }

    if ( RestServerApplication::instance()->requiresValidSession() && ( !session || session->isExpired() ) )
    {
        return std::make_tuple( http::status::forbidden, "No valid session provided", nullptr );
    }
    auto documents = RestServerApplication::instance()->documents( session.get() );
    CAFFA_DEBUG( "Found " << documents.size() << " document" );

    auto jsonResult = nlohmann::json::array();
    for ( auto document : documents )
    {
        jsonResult.push_back( createJsonFromProjectObject( document.get() ) );
    }
    return std::make_tuple( http::status::ok, jsonResult.dump(), nullptr );
}
