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
#include "cafSession.h"

#include <memory>
#include <string>
#include <utility>

namespace caffa
{
struct AppInfo;
}

namespace caffa::rpc
{
class Client
{
public:
    Client( std::string hostname, int port )
        : m_hostname( std::move( hostname ) )
        , m_port( port )
    {
    }
    virtual ~Client() = default;

    [[nodiscard]] virtual AppInfo                       appInfo() const                                 = 0;
    [[nodiscard]] virtual std::shared_ptr<ObjectHandle> document( const std::string& documentId ) const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<ObjectHandle>> documents() const                  = 0;
    [[nodiscard]] virtual std::string                                execute( not_null<const ObjectHandle*> selfObject,
                                                                              const std::string&            methodName,
                                                                              const std::string&            jsonArguments ) const = 0;
    virtual void                                                     sendKeepAlive()                     = 0;
    [[nodiscard]] virtual std::pair<bool, bool>                      isReady( Session::Type type ) const = 0;

    void createSession( Session::Type type, const std::string& username = "", const std::string& password = "" )
    {
        doCreateSession( type, username, password );
    }

    /**
     * @brief Check the session. Will return a session type (including possibly INVALID) if the session exists.
     * And throw an exception if it does not.
     * @return Session::Type
     */
    [[nodiscard]] virtual Session::Type                 checkSession() const                   = 0;
    virtual void                                        changeSession( Session::Type newType ) = 0;
    [[nodiscard]] virtual const std::string&            sessionUuid() const                    = 0;
    virtual void                                        startKeepAliveThread()                 = 0;
    [[nodiscard]] virtual std::shared_ptr<ObjectHandle> sessionMetadata()                      = 0;

    template <typename DataType>
    void set( const ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value );

    template <typename DataType>
    DataType get( const ObjectHandle* objectHandle, const std::string& fieldName ) const;

    virtual std::shared_ptr<ObjectHandle> getChildObject( const ObjectHandle* objectHandle,
                                                          const std::string&  fieldName ) const = 0;

    virtual std::vector<std::shared_ptr<ObjectHandle>> getChildObjects( const ObjectHandle* objectHandle,
                                                                        const std::string&  fieldName ) const = 0;

    virtual void setChildObject( const ObjectHandle* objectHandle,
                                 const std::string&  fieldName,
                                 const ObjectHandle* childObject ) = 0;

    virtual void removeChildObject( const ObjectHandle* objectHandle, const std::string& fieldName, size_t index ) = 0;

    virtual void clearChildObjects( const ObjectHandle* objectHandle, const std::string& fieldName ) = 0;

    virtual void insertChildObject( const ObjectHandle* objectHandle,
                                    const std::string&  fieldName,
                                    size_t              index,
                                    const ObjectHandle* childObject ) = 0;

    [[nodiscard]] const std::string& hostname() const { return m_hostname; }
    [[nodiscard]] int                port() const { return m_port; }

private:
    virtual void setJson( const ObjectHandle* objectHandle, const std::string& fieldName, const json::value& value ) = 0;
    virtual json::value getJson( const ObjectHandle*, const std::string& fieldName ) const                       = 0;
    virtual void doCreateSession( Session::Type type, const std::string& username, const std::string& password ) = 0;

private:
    std::string m_hostname;
    int         m_port;
};

//--------------------------------------------------------------------------------------------------
/// Get a value through RPC
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType Client::get( const ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    const json::value jsonValue = getJson( objectHandle, fieldName );
    return json::from_json<DataType>( jsonValue );
}

//--------------------------------------------------------------------------------------------------
/// Set a value through RPC
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void Client::set( const ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value )
{
    const json::value jsonValue = json::to_json( value );
    setJson( objectHandle, fieldName, jsonValue );
}

} // namespace caffa::rpc
