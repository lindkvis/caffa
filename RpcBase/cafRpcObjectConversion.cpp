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
#include "cafRpcObjectConversion.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafDocument.h"
#include "cafFieldHandle.h"
#include "cafFieldScriptingCapability.h"
#include "cafJsonSerializer.h"
#include "cafLogger.h"
#include "cafObject.h"
#include "cafObjectFinder.h"
#include "cafRpcServerApplication.h"

namespace caffa::rpc
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool fieldIsScriptReadable( const FieldHandle* fieldHandle )
{
    const auto scriptability = fieldHandle->capability<FieldScriptingCapability>();
    return scriptability && scriptability->isReadable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
json::object createJsonSchemaFromProjectObject( const ObjectHandle* source )
{
    CAFFA_ASSERT( source );
    CAFFA_ASSERT( !source->uuid().empty() );

    JsonSerializer serializer( DefaultObjectFactory::instance().get() );
    serializer.setSerializationType( JsonSerializer::SerializationType::SCHEMA );

    json::object object;
    serializer.writeObjectToJson( source, object );
    return object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
json::object createJsonFromProjectObject( const ObjectHandle* source )
{
    CAFFA_ASSERT( source );
    CAFFA_ASSERT( !source->uuid().empty() );

    JsonSerializer serializer( DefaultObjectFactory::instance().get() );
    serializer.setSerializeUuids( true );
    serializer.setFieldSelector( fieldIsScriptReadable );

    json::object object;
    serializer.writeObjectToJson( source, object );
    return object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
json::object createJsonSkeletonFromProjectObject( const ObjectHandle* source )
{
    CAFFA_ASSERT( source );
    CAFFA_ASSERT( !source->uuid().empty() );

    JsonSerializer serializer( DefaultObjectFactory::instance().get() );
    serializer.setFieldSelector( fieldIsScriptReadable );
    serializer.setSerializationType( JsonSerializer::SerializationType::DATA_SKELETON );
    serializer.setSerializeUuids( true );

    json::object object;
    serializer.writeObjectToJson( source, object );
    return object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> findCafObjectFromJsonObject( const Session* session, const std::string& jsonObject )
{
    CAFFA_TRACE( "Looking for object from json: " << jsonObject );
    const auto objectUuid = JsonSerializer::readUUIDFromObjectString( jsonObject );
    return findCafObjectFromUuid( session, objectUuid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ObjectHandle> findCafObjectFromUuid( const Session* session, const std::string& objectUuid )
{
    CAFFA_TRACE( "Looking for caf object with UUID '" << objectUuid << "'" );

    for ( const auto& doc : ServerApplication::instance()->documents( session ) )
    {
        ObjectFinder<> finder( [objectUuid]( const ObjectHandle* objectHandle ) -> bool
                               { return objectHandle->uuid() == objectUuid; } );
        doc->accept( &finder );
        if ( finder.object() )
        {
            return finder.object();
        }
    }
    return nullptr;
}
} // namespace caffa::rpc