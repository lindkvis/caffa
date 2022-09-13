#include "cafSession.h"

#include "cafUuidGenerator.h"

#include <random>

using namespace caffa;

Session::Session( std::chrono::milliseconds timeout )
    : m_uuid( caffa::UuidGenerator::generate() )
    , m_lastKeepAlive( std::chrono::steady_clock::now() )
    , m_timeOut( timeout )
{
}

const std::string& Session::uuid() const
{
    return m_uuid;
}

bool Session::isExpired() const
{
    std::scoped_lock<std::mutex> lock( m_mutex );

    auto now = std::chrono::steady_clock::now();
    return ( now - m_lastKeepAlive > m_timeOut );
}

void Session::updateKeepAlive()
{
    std::scoped_lock<std::mutex> lock( m_mutex );
    m_lastKeepAlive = std::chrono::steady_clock::now();
}
