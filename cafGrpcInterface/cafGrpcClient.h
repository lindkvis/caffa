#pragma once

#include "cafApplication.h"
#include "cafPdmDocument.h"

#include <memory>
#include <string>

namespace caf::rpc
{
class ClientImpl;

class Client
{
public:
    Client( const std::string& hostname, int port = 55555 );
    ~Client();

    caf::AppInfo                       appInfo() const;
    std::unique_ptr<caf::ObjectHandle> document( const std::string& documentId ) const;
    bool                               sync(caf::ObjectHandle* objectHandle );
    bool                               stopServer() const;

private:
    ClientImpl* m_clientImpl;
};
} // namespace caf::rpc
