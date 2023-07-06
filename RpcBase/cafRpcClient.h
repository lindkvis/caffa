// ##################################################################################################
//
//    Caffa
//    Copyright (C) Kontur AS
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
#pragma once

#include "cafDocument.h"
#include "cafLogger.h"
#include "cafNotNull.h"
#include "cafPortableDataType.h"
#include "cafSession.h"

#include <memory>
#include <string>

namespace caffa
{
struct AppInfo;
}

namespace caffa::rpc
{
class Client
{
public:
    virtual ~Client() = default;

    virtual caffa::AppInfo                                    appInfo() const                                 = 0;
    virtual std::shared_ptr<caffa::ObjectHandle>              document( const std::string& documentId ) const = 0;
    virtual std::vector<std::shared_ptr<caffa::ObjectHandle>> documents() const                               = 0;
    virtual std::string execute( caffa::not_null<const caffa::ObjectHandle*> selfObject,
                                 const std::string&                          jsonMethod ) const                                        = 0;
    virtual bool        stopServer()                                                                          = 0;
    virtual void        sendKeepAlive()                                                                       = 0;

    /**
     * @brief Check the session. Will return a session type (including possibly INVALID) if the session exists.
     * And throw an exception if it does not.
     * @return caffa::Session::Type
     */
    virtual caffa::Session::Type checkSession() const                          = 0;
    virtual void                 changeSession( caffa::Session::Type newType ) = 0;
    virtual void                 destroySession()                              = 0;
    virtual const std::string&   sessionUuid() const                           = 0;
    virtual void                 startKeepAliveThread()                        = 0;

    virtual bool ping() const = 0;

    template <typename DataType>
    void set( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value );

    template <typename DataType>
    DataType get( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const;

    virtual std::shared_ptr<caffa::ObjectHandle> getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                              const std::string& fieldName ) const = 0;

    virtual std::shared_ptr<caffa::ObjectHandle> getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                           const std::string& fieldName ) const    = 0;
    virtual void                                 deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                                                          const std::string&         fieldName,
                                                                          const caffa::ObjectHandle* childObject ) = 0;

    virtual std::vector<std::shared_ptr<caffa::ObjectHandle>> getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                               const std::string& fieldName ) const = 0;

    virtual void setChildObject( const caffa::ObjectHandle* objectHandle,
                                 const std::string&         fieldName,
                                 const caffa::ObjectHandle* childObject ) = 0;

    virtual void
        removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index ) = 0;

    virtual void clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) = 0;

    virtual void insertChildObject( const caffa::ObjectHandle* objectHandle,
                                    const std::string&         fieldName,
                                    size_t                     index,
                                    const caffa::ObjectHandle* childObject ) = 0;

private:
    virtual void
        setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value ) = 0;
    virtual nlohmann::json getJson( const caffa::ObjectHandle*, const std::string& fieldName ) const = 0;
};

//--------------------------------------------------------------------------------------------------
/// Get a value through gRPC
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType caffa::rpc::Client::get( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    CAFFA_DEBUG( "Got object Handle: " << objectHandle->classKeyword() );
    nlohmann::json jsonValue = getJson( objectHandle, fieldName );
    CAFFA_TRACE( "Attempting to get a value of datatype " << caffa::PortableDataType<DataType>::name()
                                                          << " from json value " << jsonValue );
    if ( jsonValue.is_object() && jsonValue.contains( "value" ) )
    {
        return jsonValue["value"].get<DataType>();
    }
    return jsonValue.get<DataType>();
}

//--------------------------------------------------------------------------------------------------
/// Set a value through gRPC
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value )
{
    nlohmann::json jsonValue = value;
    setJson( objectHandle, fieldName, jsonValue );
}

} // namespace caffa::rpc
