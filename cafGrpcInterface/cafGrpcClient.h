//##################################################################################################
//
//   Caffa
//   Copyright (C) Gaute Lindkvist
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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

#include "cafApplication.h"
#include "cafObjectMethod.h"
#include "cafPdmDocument.h"
#include "cafVariant.h"

#include <gsl/gsl>

#include <memory>
#include <string>

namespace caf::rpc
{
class ClientImpl;

class Client
{
public:
    Client( const std::string& hostname, int port = 55555 );
    virtual ~Client();

    caf::AppInfo                       appInfo() const;
    std::unique_ptr<caf::ObjectHandle> document( const std::string& documentId ) const;
    std::unique_ptr<caf::ObjectHandle> execute( gsl::not_null<const caf::ObjectMethod*> method ) const;
    bool                               stopServer() const;

    template <typename DataType>
    void set( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value );

    template <typename DataType>
    DataType get( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const;

private:
    void setJson( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const nlohmann::json& value );
    nlohmann::json getJson( const caf::ObjectHandle*, const std::string& fieldName ) const;

private:
    std::unique_ptr<ClientImpl> m_clientImpl;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType caf::rpc::Client::get( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const
{
    nlohmann::json jsonValue = getJson( objectHandle, fieldName );
    return jsonValue.get<DataType>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::rpc::Client::set( const caf::ObjectHandle* objectHandle, const std::string& fieldName, const DataType& value )
{
    nlohmann::json jsonValue = value;
    setJson( objectHandle, fieldName, jsonValue );
}

template <>
void Client::set<std::vector<int>>( const caf::ObjectHandle* objectHandle,
                                    const std::string&       fieldName,
                                    const std::vector<int>&  value );

template <>
void Client::set<std::vector<double>>( const caf::ObjectHandle*   objectHandle,
                                       const std::string&         fieldName,
                                       const std::vector<double>& value );

template <>
void Client::set<std::vector<float>>( const caf::ObjectHandle*  objectHandle,
                                      const std::string&        fieldName,
                                      const std::vector<float>& value );

template <>
void Client::set<std::vector<std::string>>( const caf::ObjectHandle*        objectHandle,
                                            const std::string&              fieldName,
                                            const std::vector<std::string>& value );

template <>
std::vector<int> Client::get<std::vector<int>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const;

template <>
std::vector<double>
    Client::get<std::vector<double>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const;

template <>
std::vector<float>
    Client::get<std::vector<float>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const;

template <>
std::vector<std::string>
    Client::get<std::vector<std::string>>( const caf::ObjectHandle* objectHandle, const std::string& fieldName ) const;

} // namespace caf::rpc
