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

#include "cafDocument.h"
#include "cafLogger.h"
#include "cafNotNull.h"
#include "cafPortableDataType.h"
#include "cafRpcClient.h"
#include "cafSession.h"

#include <memory>
#include <string>

namespace caffa
{
struct AppInfo;
}

namespace caffa::rpc
{
class GrpcClientImpl;

class GrpcClient : public Client
{
public:
    GrpcClient( caffa::Session::Type sessionType,
                const std::string&   hostname,
                int                  port           = 50000,
                const std::string&   clientCertFile = "",
                const std::string&   clientKeyFile  = "",
                const std::string&   caCertFile     = "" );
    ~GrpcClient() override;

    caffa::AppInfo                                    appInfo() const override;
    std::shared_ptr<caffa::ObjectHandle>              document( const std::string& documentId ) const override;
    std::vector<std::shared_ptr<caffa::ObjectHandle>> documents() const;
    std::string                                       execute( caffa::not_null<const caffa::ObjectHandle*> selfObject,
                                                               const std::string&                          methodName,
                                                               const std::string&                          jsonArguments ) const override;
    bool                                              stopServer() override;
    void                                              sendKeepAlive() override;

    /**
     * @brief Check the session. Will return a session type (including possibly INVALID) if the session exists.
     * And throw an exception if it does not.
     * @return caffa::Session::Type
     */
    caffa::Session::Type checkSession() const override;
    void                 changeSession( caffa::Session::Type newType ) override;
    void                 destroySession() override;
    const std::string&   sessionUuid() const override;
    void                 startKeepAliveThread() override;

    bool ping() const override;

    std::shared_ptr<caffa::ObjectHandle> getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                      const std::string& fieldName ) const override;

    std::shared_ptr<caffa::ObjectHandle> getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                   const std::string& fieldName ) const override;

    void deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                  const std::string&         fieldName,
                                  const caffa::ObjectHandle* childObject ) override;

    std::vector<std::shared_ptr<caffa::ObjectHandle>> getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                       const std::string& fieldName ) const override;

    void setChildObject( const caffa::ObjectHandle* objectHandle,
                         const std::string&         fieldName,
                         const caffa::ObjectHandle* childObject ) override;

    void removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index ) override;

    void clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName ) override;

    void insertChildObject( const caffa::ObjectHandle* objectHandle,
                            const std::string&         fieldName,
                            size_t                     index,
                            const caffa::ObjectHandle* childObject ) override;

private:
    void setJson( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value ) override;
    nlohmann::json getJson( const caffa::ObjectHandle*, const std::string& fieldName ) const override;

private:
    std::unique_ptr<GrpcClientImpl> m_clientImpl;
};

} // namespace caffa::rpc
