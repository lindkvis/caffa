#pragma once

#include "cafApplication.h"

#include <memory>
#include <string>

namespace caf::rpc
{
class Client;
}

namespace caf
{
class GrpcClientApplication : public Application
{
public:
    GrpcClientApplication( const std::string& hostname, int portNumber );
    static GrpcClientApplication* instance();

private:
    std::unique_ptr<caf::rpc::Client> m_client;
};
} // namespace caf
