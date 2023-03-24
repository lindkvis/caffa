// ##################################################################################################
//
//    Caffa
//    Copyright (C) Gaute Lindkvist
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
#include "cafObjectMethod.h"
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
class ClientImpl;

class Client
{
public:
    Client( caffa::Session::Type sessionType,
            const std::string&   hostname,
            int                  port           = 50000,
            const std::string&   clientCertFile = "",
            const std::string&   clientKeyFile  = "",
            const std::string&   caCertFile     = "" );
    virtual ~Client();

    caffa::AppInfo                                    appInfo() const;
    std::unique_ptr<caffa::ObjectHandle>              document( const std::string& documentId ) const;
    std::vector<std::unique_ptr<caffa::ObjectHandle>> documents() const;
    std::unique_ptr<caffa::ObjectMethodResult> execute( caffa::not_null<const caffa::ObjectMethod*> method ) const;
    bool                                       stopServer();
    void                                       sendKeepAlive();

    /**
     * @brief Check the session. Will return a session type (including possibly INVALID) if the session exists.
     * And throw an exception if it does not.
     * @return caffa::Session::Type
     */
    caffa::Session::Type checkSession() const;
    void                 changeSession( caffa::Session::Type newType );
    void                 destroySession();
    const std::string&   sessionUuid() const;
    void                 startKeepAliveThread();

    bool                                            ping() const;
    std::list<std::unique_ptr<caffa::ObjectHandle>> objectMethods( caffa::ObjectHandle* objectHandle ) const;

    template <typename DataType>
    void set( const caffa::ObjectHandle* objectHandle,
              const std::string&         fieldName,
              const DataType&            value,
              uint32_t                   addressOffset = 0u );

    template <typename DataType>
    DataType get( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, uint32_t addressOffset = 0u ) const;

    std::unique_ptr<caffa::ObjectHandle> getShallowCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                      const std::string&         fieldName ) const;

    std::unique_ptr<caffa::ObjectHandle> getDeepCopyOfChildObject( const caffa::ObjectHandle* objectHandle,
                                                                   const std::string&         fieldName ) const;

    void deepCopyChildObjectFrom( const caffa::ObjectHandle* objectHandle,
                                  const std::string&         fieldName,
                                  const caffa::ObjectHandle* childObject );

    std::vector<std::unique_ptr<caffa::ObjectHandle>> getChildObjects( const caffa::ObjectHandle* objectHandle,
                                                                       const std::string&         fieldName ) const;

    void setChildObject( const caffa::ObjectHandle* objectHandle,
                         const std::string&         fieldName,
                         const caffa::ObjectHandle* childObject );

    void removeChildObject( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, size_t index );

    void clearChildObjects( const caffa::ObjectHandle* objectHandle, const std::string& fieldName );

    void insertChildObject( const caffa::ObjectHandle* objectHandle,
                            const std::string&         fieldName,
                            size_t                     index,
                            const caffa::ObjectHandle* childObject );

private:
    void           setJson( const caffa::ObjectHandle* objectHandle,
                            const std::string&         fieldName,
                            const nlohmann::json&      value,
                            uint32_t                   addressOffset );
    nlohmann::json getJson( const caffa::ObjectHandle*, const std::string& fieldName, uint32_t addressOffset ) const;

private:
    std::unique_ptr<ClientImpl> m_clientImpl;
};

//--------------------------------------------------------------------------------------------------
/// Get a value through gRPC. Note the addressOffset is only relevant for register fields
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType caffa::rpc::Client::get( const caffa::ObjectHandle* objectHandle,
                                  const std::string&         fieldName,
                                  uint32_t                   addressOffset ) const
{
    CAFFA_DEBUG( "Got object Handle: " << objectHandle->classKeyword() );
    nlohmann::json jsonValue = getJson( objectHandle, fieldName, addressOffset );
    CAFFA_TRACE( "Attempting to get a value of datatype " << caffa::PortableDataType<DataType>::name()
                                                          << " from json value " << jsonValue );
    if ( jsonValue.is_object() && jsonValue.contains( "value" ) )
    {
        return jsonValue["value"].get<DataType>();
    }
    return jsonValue.get<DataType>();
}

//--------------------------------------------------------------------------------------------------
/// Set a value through gRPC. Note the addressOffset is only relevant for register fields
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caffa::rpc::Client::set( const caffa::ObjectHandle* objectHandle,
                              const std::string&         fieldName,
                              const DataType&            value,
                              uint32_t                   addressOffset )
{
    nlohmann::json jsonValue = value;
    setJson( objectHandle, fieldName, jsonValue, addressOffset );
}

} // namespace caffa::rpc
