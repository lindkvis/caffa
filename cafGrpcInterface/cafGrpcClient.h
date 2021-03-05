#pragma once

#include "cafApplication.h"
#include "cafObjectMethod.h"
#include "cafPdmDocument.h"

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
    bool                               sync( gsl::not_null<caf::ObjectHandle*> objectHandle );
    bool                               stopServer() const;

private:
    std::unique_ptr<ClientImpl> m_clientImpl;
};
} // namespace caf::rpc
