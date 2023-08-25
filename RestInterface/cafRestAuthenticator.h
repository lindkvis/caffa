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
// ##################################################################################################

#pragma once

#include <string>

namespace caffa::rpc
{

/**
 * An abstract autheticator which takes the HTTP authorisation header line
 * and checks if this is acceptable.
 * CAFFA only comes with an completly non-production-ready Demo Authenticator
 * in the Examples section.
 * Application developers have to implement their own (secure) authenticator.
 * If any of the SSL-methods return a blank string, SSL will be disabled.
 */
class RestAuthenticator
{
public:
    virtual ~RestAuthenticator() = default;

    /**
     * A SSL certificate. If left blank SSL will be disabled.
     */
    virtual std::string sslCertificate() const = 0;

    /**
     * A SSL private key. If left blank SSL will be disabled.
     */
    virtual std::string sslKey() const = 0;

    /**
     * SSL Diffie-Hellman parameters. If left blank SSL will be disabled.
     */
    virtual std::string sslDhParameters() const = 0;

    /**
     * Perform authentication with the HTTP authorisation header line.
     * @param authorisationHeader The authorisation line. Contains both username and password separated by a colon.
     */
    virtual bool authenticate( const std::string& authorisationHeader ) const = 0;
};
} // namespace caffa::rpc