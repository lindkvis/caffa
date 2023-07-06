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
#include "cafObject.h"
#include "cafObjectCollector.h"
#include "cafRpcServerApplication.h"

using namespace caffa::rpc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool fieldIsScriptReadable( const caffa::FieldHandle* fieldHandle )
{
    auto scriptability = fieldHandle->capability<caffa::FieldScriptingCapability>();
    return scriptability && scriptability->isReadable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool fieldIsScriptWritable( const caffa::FieldHandle* fieldHandle )
{
    auto scriptability = fieldHandle->capability<caffa::FieldScriptingCapability>();
    return scriptability && scriptability->isWritable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caffa::rpc::createJsonSelfReferenceFromCaf( const caffa::ObjectHandle* source )
{
    CAFFA_ASSERT( source );
    CAFFA_ASSERT( !source->uuid().empty() );

    caffa::JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    serializer.setFieldSelector( fieldIsScriptReadable );
    serializer.setWriteTypesAndValidators( false );
    serializer.setSerializeUuids( true );
    serializer.setSerializeDataTypes( false );

    std::string jsonString = serializer.writeObjectToString( source );
    CAFFA_TRACE( jsonString );
    return jsonString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string caffa::rpc::createJsonFromProjectObject( const caffa::ObjectHandle* source )
{
    CAFFA_ASSERT( source );
    CAFFA_ASSERT( !source->uuid().empty() );

    caffa::JsonSerializer serializer( caffa::DefaultObjectFactory::instance() );
    serializer.setFieldSelector( fieldIsScriptWritable );
    serializer.setWriteTypesAndValidators( false );
    serializer.setSerializeUuids( true );

    std::string jsonString = serializer.writeObjectToString( source );
    return jsonString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::ObjectHandle* caffa::rpc::findCafObjectFromJsonObject( const caffa::Session* session, const std::string& jsonObject )
{
    CAFFA_TRACE( "Looking for object from json: " << jsonObject );
    auto [classKeyword, objectUuid] = caffa::JsonSerializer().readClassKeywordAndUUIDFromObjectString( jsonObject );
    return findCafObjectFromScriptNameAndUuid( session, classKeyword, objectUuid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::ObjectHandle* caffa::rpc::findCafObjectFromScriptNameAndUuid( const caffa::Session* session,
                                                                     const std::string&    scriptClassName,
                                                                     const std::string&    objectUuid )
{
    CAFFA_TRACE( "Looking for caf object with class name '" << scriptClassName << "' and UUID '" << objectUuid << "'" );

    caffa::ObjectCollector<> collector(
        [scriptClassName]( const caffa::ObjectHandle* objectHandle ) -> bool
        { return caffa::ObjectHandle::matchesClassKeyword( scriptClassName, objectHandle->classInheritanceStack() ); } );

    for ( auto doc : ServerApplication::instance()->documents( session ) )
    {
        doc->accept( &collector );
    }

    for ( caffa::ObjectHandle* testObject : collector.objects() )
    {
        CAFFA_TRACE( "Testing object with class name '" << testObject->classKeyword() << "' and UUID '"
                                                        << testObject->uuid() << "'" );

        if ( testObject && testObject->uuid() == objectUuid )
        {
            return testObject;
        }
    }

    return nullptr;
}
