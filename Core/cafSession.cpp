#include "cafSession.h"

#include "cafUuidGenerator.h"

#include <random>

using namespace caffa;

Session::Session( Type type, std::chrono::milliseconds timeout )
    : m_uuid( caffa::UuidGenerator::generate() )
    , m_type( type )
    , m_lastKeepAlive( std::chrono::steady_clock::now() )
    , m_timeOut( timeout )
{
}

const std::string& Session::uuid() const
{
    return m_uuid;
}

Session::Type Session::type() const
{
    return m_type;
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

Session::Type Session::typeFromUint( unsigned type )
{
    switch ( type )
    {
        case static_cast<unsigned>( Session::Type::REGULAR ):
            return Session::Type::REGULAR;
        case static_cast<unsigned>( Session::Type::OBSERVING ):
            return Session::Type::OBSERVING;
        default:
            throw std::runtime_error( "Bad session type " + std::to_string( type ) );
    }
    return Session::Type::INVALID;
}
