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
#pragma once

#include "cafJsonDefinitions.h"
#include "cafNotNull.h"
#include "cafRpcClient.h"
#include "cafSession.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

namespace caffa
{
struct AppInfo;
class ObjectHandle;
} // namespace caffa

namespace http = boost::beast::http; // from <boost/beast/http.hpp>

namespace boost::asio
{
class io_context;
}

namespace caffa::rpc
{
class RestClient : public Client
{
public:
    explicit RestClient( const std::string&               hostname,
                         int                              port    = 50000,
                         const std::chrono::milliseconds& timeout = std::chrono::seconds( 10 ) );

    ~RestClient() override;

    AppInfo                                    appInfo() const override;
    std::shared_ptr<ObjectHandle>              document( const std::string& documentId ) const override;
    std::vector<std::shared_ptr<ObjectHandle>> documents() const override;
    std::string                                execute( not_null<const ObjectHandle*> selfObject,
                                                        const std::string&            methodName,
                                                        const std::string&            jsonArguments ) const override;
    void                                       sendKeepAlive() override;
    bool                                       isReady( Session::Type type ) const override;

    /**
     * @brief Check the session. Will return a session type (including possibly INVALID) if the session exists.
     * And throw an exception if it does not.
     * @return Session::Type
     */
    Session::Type                 checkSession() const override;
    void                          changeSession( Session::Type newType ) override;
    void                          destroySession();
    const std::string&            sessionUuid() const override;
    void                          startKeepAliveThread() override;
    std::shared_ptr<ObjectHandle> sessionMetadata() override;

    std::shared_ptr<ObjectHandle> getChildObject( const ObjectHandle* objectHandle,
                                                  const std::string&  fieldName ) const override;

    std::vector<std::shared_ptr<ObjectHandle>> getChildObjects( const ObjectHandle* objectHandle,
                                                                const std::string&  fieldName ) const override;

    void setChildObject( const ObjectHandle* objectHandle,
                         const std::string&  fieldName,
                         const ObjectHandle* childObject ) override;

    void removeChildObject( const ObjectHandle* objectHandle, const std::string& fieldName, size_t index ) override;

    void clearChildObjects( const ObjectHandle* objectHandle, const std::string& fieldName ) override;

    void insertChildObject( const ObjectHandle* objectHandle,
                            const std::string&  fieldName,
                            size_t              index,
                            const ObjectHandle* childObject ) override;

private:
    void doCreateSession( Session::Type type, const std::string& username, const std::string& password ) override;

    void setJson( const ObjectHandle* objectHandle, const std::string& fieldName, const json::value& value ) override;
    json::value getJson( const ObjectHandle*, const std::string& fieldName ) const override;

    std::pair<http::status, std::string> performRequest( http::verb         verb,
                                                         const std::string& hostname,
                                                         int                port,
                                                         const std::string& target,
                                                         const std::string& body,
                                                         const std::string& username = "",
                                                         const std::string& password = "" ) const;
    std::pair<http::status, std::string> performGetRequest( const std::string& hostname,
                                                            int                port,
                                                            const std::string& target,
                                                            const std::string& username = "",
                                                            const std::string& password = "" ) const;

private:
    std::chrono::milliseconds m_timeout;
    std::string               m_sessionUuid;
    std::thread               m_keepAliveThread;
    mutable std::mutex        m_sessionMutex;

    // The io_context is required for all I/O
    std::shared_ptr<boost::asio::io_context>          m_ioc;
    mutable std::shared_ptr<boost::beast::tcp_stream> m_stream;
};

} // namespace caffa::rpc
