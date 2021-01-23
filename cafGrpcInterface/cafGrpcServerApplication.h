#pragma once

#include "cafApplication.h"

#include <memory>

namespace caf::rpc
{
class Server;
}

namespace caf
{
class GrpcServerApplication : public Application
{
public:
    GrpcServerApplication( int portNumber );
    static GrpcServerApplication* instance();

    void run();
    bool running() const;
    void quit();

private:
    std::unique_ptr<caf::rpc::Server> m_server;
};
} // namespace caf
