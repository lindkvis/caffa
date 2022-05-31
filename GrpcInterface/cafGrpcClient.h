//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#pragma once

#include "cafDocument.h"
#include "cafLogger.h"
#include "cafNotNull.h"
#include "cafObjectMethod.h"
#include "cafPortableDataType.h"

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
    Client( const std::string& hostname,
            int                port           = 50000,
            const std::string& clientCertFile = "",
            const std::string& clientKeyFile  = "",
            const std::string& caCertFile     = "" );
    virtual ~Client();

    caffa::AppInfo                                    appInfo() const;
    std::unique_ptr<caffa::ObjectHandle>              document( const std::string& documentId ) const;
    std::vector<std::unique_ptr<caffa::ObjectHandle>> documents() const;
    std::unique_ptr<caffa::ObjectMethodResult> execute( caffa::not_null<const caffa::ObjectMethod*> method ) const;
    bool                                       stopServer();
    void                                       sendKeepAlive();
    void                                       destroySession();
    const std::string&                         sessionUuid() const;

    bool                                            ping() const;
    void                                            resetToDefaultData() const;
    std::list<std::unique_ptr<caffa::ObjectHandle>> objectMethods( caffa::ObjectHandle* objectHandle ) const;

    template <typename DataType>
    void set( const caffa::ObjectHandle* objectHandle,
              const std::string&         fieldName,
              const DataType&            value,
              uint32_t                   addressOffset = 0u );

    template <typename DataType>
    DataType get( const caffa::ObjectHandle* objectHandle, const std::string& fieldName, uint32_t addressOffset = 0u ) const;

    std::unique_ptr<caffa::ObjectHandle> getChildObject( const caffa::ObjectHandle* objectHandle,
                                                         const std::string&         fieldName ) const;

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

template <>
void Client::set<std::vector<int>>( const caffa::ObjectHandle* objectHandle,
                                    const std::string&         fieldName,
                                    const std::vector<int>&    value,
                                    uint32_t                   addressOffset );

template <>
void Client::set<std::vector<uint64_t>>( const caffa::ObjectHandle*   objectHandle,
                                         const std::string&           fieldName,
                                         const std::vector<uint64_t>& value,
                                         uint32_t                     addressOffset );

template <>
void Client::set<std::vector<double>>( const caffa::ObjectHandle* objectHandle,
                                       const std::string&         fieldName,
                                       const std::vector<double>& value,
                                       uint32_t                   addressOffset );

template <>
void Client::set<std::vector<float>>( const caffa::ObjectHandle* objectHandle,
                                      const std::string&         fieldName,
                                      const std::vector<float>&  value,
                                      uint32_t                   addressOffset );

template <>
void Client::set<std::vector<std::string>>( const caffa::ObjectHandle*      objectHandle,
                                            const std::string&              fieldName,
                                            const std::vector<std::string>& value,
                                            uint32_t                        addressOffset );

template <>
std::vector<int> Client::get<std::vector<int>>( const caffa::ObjectHandle* objectHandle,
                                                const std::string&         fieldName,
                                                uint32_t                   addressOffset ) const;

template <>
std::vector<int> Client::get<std::vector<int>>( const caffa::ObjectHandle* objectHandle,
                                                const std::string&         fieldName,
                                                uint32_t                   addressOffset ) const;

template <>
std::vector<uint64_t> Client::get<std::vector<uint64_t>>( const caffa::ObjectHandle* objectHandle,
                                                          const std::string&         fieldName,
                                                          uint32_t                   addressOffset ) const;

template <>
std::vector<double> Client::get<std::vector<double>>( const caffa::ObjectHandle* objectHandle,
                                                      const std::string&         fieldName,
                                                      uint32_t                   addressOffset ) const;

template <>
std::vector<float> Client::get<std::vector<float>>( const caffa::ObjectHandle* objectHandle,
                                                    const std::string&         fieldName,
                                                    uint32_t                   addressOffset ) const;

template <>
std::vector<std::string> Client::get<std::vector<std::string>>( const caffa::ObjectHandle* objectHandle,
                                                                const std::string&         fieldName,
                                                                uint32_t                   addressOffset ) const;

} // namespace caffa::rpc
