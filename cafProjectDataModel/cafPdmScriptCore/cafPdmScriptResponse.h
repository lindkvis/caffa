//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
//##################################################################################################
#pragma once

#include "cafObject.h"
#include "cafPdmPointer.h"

#include <list>
#include <string>

#include <memory>

namespace caf
{
//==================================================================================================
//
// Command response which contains status and possibly a result
//
//==================================================================================================
class PdmScriptResponse
{
public:
    // Status in order of severity from ok to critical
    enum Status
    {
        COMMAND_OK,
        COMMAND_WARNING,
        COMMAND_ERROR
    };

public:
    PdmScriptResponse( Status status = COMMAND_OK, const std::string& message = "" );
    explicit PdmScriptResponse( Object* ok_result );

    Status                 status() const;
    std::string            sanitizedResponseMessage() const;
    std::list<std::string> messages() const;
    Object*                result() const;
    void                   setResult( Object* result );
    void                   updateStatus( Status status, const std::string& message );

private:
    static std::string statusLabel( Status status );

private:
    Status                  m_status;
    std::list<std::string>  m_messages;
    std::unique_ptr<Object> m_result;
};
} // namespace caf
