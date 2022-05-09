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

#include <grpcpp/grpcpp.h>

#include <stdexcept>
#include <string>

namespace caffa::rpc
{
class Exception : public std::runtime_error
{
public:
    Exception( grpc::Status status )
        : std::runtime_error( status.error_message() )
        , m_status( status )
    {
    }

    const grpc::Status& status() const { return m_status; }

private:
    grpc::Status m_status;
};
} // namespace caffa::rpc
