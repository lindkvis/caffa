
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
#include "cafSession.h"

#include "cafDocument.h"
#include "cafField.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafObjectPerformer.h"
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

    auto skeletonParameter = std::make_unique<RestTypedParameter<bool>>( "skeleton",
                                                                         RestParameter::Location::QUERY,
                                                                         false,
                                                                         "Whether to only send the skeleton" );
    skeletonParameter->setDefaultValue( false );

    for ( const auto& document : documents )
    {
        const std::string& id = document->id();
        auto               getAction =
            std::make_unique<RestAction>( http::verb::get,
                                          "Get " + document->id() + " document",
                                          "getDocument",
                                          [id]( const http::verb              verb,
                                                const std::list<std::string>& pathArguments,
                                                const json::object&           queryParams,
                                                const json::value&            body ) {
                                              return RestDocumentService::document( id, verb, pathArguments, queryParams, body );
                                          } );

        getAction->addResponse( http::status::ok,
                                RestResponse::objectResponse( "#/components/object_schemas/" +
                                                                  std::string( document->classKeyword() ),
                                                              document->classDocumentation() ) );

        getAction->addResponse( http::status::unknown, RestResponse::plainErrorResponse() );
        getAction->addParameter( skeletonParameter->clone() );
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
    getAllAction->addParameter( skeletonParameter->clone() );

    getAllAction->setRequiresAuthentication( false );
    getAllAction->setRequiresSession( true );

    m_requestPathRoot->addAction( std::move( getAllAction ) );
}

RestDocumentService::ServiceResponse RestDocumentService::perform( const http::verb       verb,
                                                                   std::list<std::string> path,
                                                                   const json::object&    queryParams,
                                                                   const json::value&     body )
{
    CAFFA_ASSERT( !path.empty() );

    auto [request, pathArguments] = m_requestPathRoot->findPathEntry( path );
    if ( !request )
    {
        return std::make_pair( http::status::bad_request,
                               "Document Path not found: " + StringTools::join( path.begin(), path.end(), "/" ) );
    }

    return request->perform( verb, pathArguments, queryParams, body );
}

//--------------------------------------------------------------------------------------------------
/// The object service uses session uuids to decide if it accepts the request or not
//--------------------------------------------------------------------------------------------------
bool RestDocumentService::requiresAuthentication( http::verb, const std::list<std::string>& ) const
{
    return false;
}

bool RestDocumentService::requiresSession( http::verb, const std::list<std::string>& ) const
{
    return true;
}

class PathCreator final : public Inspector
{
public:
    PathCreator()
        : m_pathStack( { "/documents" } )
    {
        m_serializer.setSerializationType( JsonSerializer::SerializationType::PATH );
    }

    const std::map<std::string, json::object>& pathSchemas() const { return m_pathSchemas; }

    void visit( const std::shared_ptr<const ObjectHandle>& object ) override
    {
        if ( const auto doc = dynamic_cast<const Document*>( object.get() ); doc )
        {
            m_pathStack.push_back( doc->id() );
            auto schema = json::object();
            m_serializer.writeObjectToJson( object.get(), schema );
            auto path           = StringTools::join( m_pathStack.begin(), m_pathStack.end(), "/" );
            m_pathSchemas[path] = schema;
        }

        for ( const auto field : object->fields() )
        {
            field->accept( this );
        }

        if ( const auto doc = dynamic_cast<const Document*>( object.get() ); doc )
        {
            m_pathStack.pop_back();
        }
    }

    void visit( const ChildFieldBaseHandle* field ) override
    {
        visitField( field );

        for ( const auto& object : field->childObjects() )
        {
            object->accept( this );
        }

        leaveField( field );
    }

    void visit( const DataField* field ) override
    {
        visitField( field );
        leaveField( field );
    }

    void visitField( const FieldHandle* field )
    {
        m_pathStack.push_back( field->keyword() );
        auto schema = json::object();

        if ( const auto scriptability = field->capability<FieldScriptingCapability>(); scriptability )
        {
            const auto jsonCapability = field->capability<FieldIoCapability>();
            if ( !jsonCapability ) return;

            if ( scriptability->isReadable() )
            {
                auto operationId = field->keyword();
                operationId[0]   = static_cast<char>( std::toupper( operationId[0] ) );
                operationId      = std::string( field->ownerObject()->classKeyword() ) + ".get" + operationId;

                json::object getOperation = { { "summary", "Get " + field->keyword() }, { "operationId", operationId } };

                const json::object fieldContent = { { "application/json", { { "schema", jsonCapability->jsonType() } } } };

                const json::object fieldResponse = { { "description", field->documentation() },
                                                     { "content", fieldContent } };

                getOperation["responses"] = fieldResponse;
                schema["get"]             = getOperation;
            }
            if ( scriptability->isWritable() )
            {
                auto operationId = field->keyword();
                operationId[0]   = static_cast<char>( std::toupper( operationId[0] ) );
                operationId      = std::string( field->ownerObject()->classKeyword() ) + ".set" + operationId;

                json::object setOperation = { { "summary", "Set " + field->keyword() }, { "operationId", operationId } };

                const json::object fieldContent = { { "application/json", { { "schema", jsonCapability->jsonType() } } } };

                json::object acceptedOrFailureResponses;

                acceptedOrFailureResponses[RestServiceInterface::HTTP_ACCEPTED] =
                    { { "description", json::to_json( "Success" ) },
                      { "default", RestServiceInterface::plainErrorResponse() } };

                setOperation["responses"]   = acceptedOrFailureResponses;
                setOperation["requestBody"] = { { "content", fieldContent } };
                schema["set"]               = setOperation;
            }
        }

        const auto path     = StringTools::join( m_pathStack.begin(), m_pathStack.end(), "/" );
        m_pathSchemas[path] = schema;
    }

    void leaveField( const FieldHandle* ) { m_pathStack.pop_back(); }

private:
    std::list<std::string> m_pathStack;
    JsonSerializer         m_serializer;

    std::map<std::string, json::object> m_pathSchemas{};
};

std::map<std::string, json::object> RestDocumentService::servicePathEntries() const
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

std::map<std::string, json::object> RestDocumentService::serviceComponentEntries() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestServiceInterface::ServiceResponse RestDocumentService::document( const std::string& documentId,
                                                                     http::verb,
                                                                     const std::list<std::string>&,
                                                                     const json::object& queryParams,
                                                                     const json::value& )
{
    std::shared_ptr<Session> session;

    if ( const auto it = queryParams.find( "session_uuid" ); it != queryParams.end() )
    {
        session = RestServerApplication::instance()->getExistingSession( json::from_json<std::string>( it->value() ) );
    }

    if ( RestServerApplication::instance()->requiresValidSession() && ( !session || session->isExpired() ) )
    {
        return std::make_pair( http::status::forbidden, "No valid session provided" );
    }

    CAFFA_TRACE( "Got document request for " << documentId );

    auto document = RestServerApplication::instance()->document( documentId, session.get() );
    if ( document )
    {
        CAFFA_TRACE( "Found document with UUID: " << document->uuid() );
        json::object jsonDocument;
        if ( const auto it = queryParams.find( "skeleton" ); it != queryParams.end() && it->value().as_bool() )
        {
            jsonDocument = createJsonSkeletonFromProjectObject( document.get() );
        }
        else
        {
            jsonDocument = createJsonFromProjectObject( document.get() );
        }
        return std::make_pair( http::status::ok, json::dump( jsonDocument ) );
    }
    return std::make_pair( http::status::not_found, "Document " + documentId + " not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RestDocumentService::ServiceResponse RestDocumentService::documents( http::verb,
                                                                     const std::list<std::string>&,
                                                                     const json::object& queryParams,
                                                                     const json::value& )
{
    CAFFA_DEBUG( "Got list document request" );

    std::shared_ptr<Session> session;

    if ( const auto it = queryParams.find( "session_uuid" ); it != queryParams.end() )
    {
        session = RestServerApplication::instance()->getExistingSession( json::from_json<std::string>( it->value() ) );
    }

    if ( RestServerApplication::instance()->requiresValidSession() && ( !session || session->isExpired() ) )
    {
        return std::make_pair( http::status::forbidden, "No valid session provided" );
    }

    const auto documents = RestServerApplication::instance()->documents( session.get() );
    CAFFA_DEBUG( "Found " << documents.size() << " document" );

    auto jsonResult = json::array();

    if ( const auto it = queryParams.find( "skeleton" ); it != queryParams.end() && it->value().as_bool() )
    {
        for ( const auto& document : documents )
        {
            jsonResult.push_back( createJsonSkeletonFromProjectObject( document.get() ) );
        }
    }
    else
    {
        for ( const auto& document : documents )
        {
            jsonResult.push_back( createJsonFromProjectObject( document.get() ) );
        }
    }

    return std::make_pair( http::status::ok, json::dump( jsonResult ) );
}
