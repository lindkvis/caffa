#include "cafSession.h"

#include "cafUuidGenerator.h"

#include "cafAssert.h"
#include "cafLogger.h"

#include <random>

using namespace caffa;

std::shared_ptr<Session> Session::create( Type type, std::chrono::milliseconds timeout )
{
    return std::shared_ptr<Session>( new Session( type, timeout ) );
}

Session::Session( Type type, std::chrono::milliseconds timeout )
    : m_uuid( caffa::UuidGenerator::generate() )
    , m_type( type )
    , m_timeOut( timeout )
    , m_lastKeepAlive( std::chrono::steady_clock::now() )
    , m_expirationBlocked( false )
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

    auto now                = std::chrono::steady_clock::now();
    auto timeSinceKeepalive = now - m_lastKeepAlive;

    if ( m_expirationBlocked )
    {
        CAFFA_DEBUG( "Session " << m_uuid << " is currently blocked from expiring without a keepalive for "
                                << timeSinceKeepalive.count() / 1000.0 << " ms. Trying with a longer timeout" );
        return timeSinceKeepalive > 4 * m_timeOut;
    }
    return timeSinceKeepalive > m_timeOut;
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

void Session::blockExpiration() const
{
    std::scoped_lock<std::mutex> lock( m_mutex );
    m_lastKeepAlive     = std::chrono::steady_clock::now();
    m_expirationBlocked = true;
}

void Session::unblockExpiration() const
{
    std::scoped_lock<std::mutex> lock( m_mutex );
    m_lastKeepAlive     = std::chrono::steady_clock::now();
    m_expirationBlocked = false;
}

SessionMaintainer::SessionMaintainer( std::shared_ptr<Session> session )
    : m_session( session )
{
    if ( m_session )
    {
        m_session->blockExpiration();
    }
}

SessionMaintainer::~SessionMaintainer()
{
    if ( m_session )
    {
        m_session->unblockExpiration();
    }
}

std::shared_ptr<Session> SessionMaintainer::operator->()
{
    return m_session;
}

SessionMaintainer::operator bool() const
{
    return !!m_session;
}

bool SessionMaintainer::operator!() const
{
    return !m_session;
}

Session* SessionMaintainer::get()
{
    return m_session.get();
}

ConstSessionMaintainer::ConstSessionMaintainer( std::shared_ptr<const Session> session )
    : m_session( session )
{
    if ( m_session )
    {
        m_session->blockExpiration();
    }
}

ConstSessionMaintainer::~ConstSessionMaintainer()
{
    if ( m_session )
    {
        m_session->unblockExpiration();
    }
}

std::shared_ptr<const Session> ConstSessionMaintainer::operator->() const
{
    return m_session;
}

ConstSessionMaintainer::operator bool() const
{
    return !!m_session;
}

bool ConstSessionMaintainer::operator!() const
{
    return !m_session;
}

const Session* ConstSessionMaintainer::get() const
{
    return m_session.get();
}