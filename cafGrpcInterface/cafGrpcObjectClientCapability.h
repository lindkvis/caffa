#pragma once

#include "cafObjectCapability.h"

#include <cstdint>

namespace caf
{
namespace rpc
{
    class ObjectService;

    class ObjectClientCapability : public caf::ObjectCapability
    {
    public:
        ObjectClientCapability( uint64_t addressOnServer = 0u)
            : m_addressOnServer( addressOnServer )
        {
        }
        uint64_t addressOnServer() const { return m_addressOnServer; }

    private:
        friend class ObjectService;

        void     setAddressOnServer( uint64_t address ) { m_addressOnServer = address; }
    private:
        uint64_t m_addressOnServer;
    };

} // namespace rpc
}

