#include "cafSession.h"

#include "cafUuidGenerator.h"

#include "cafAppEnum.h"
#include "cafAssert.h"
#include "cafLogger.h"

#include <random>

using namespace caffa;

namespace caffa
{
template <>
void AppEnum<Session::Type>::setUp()
{
    addItem( Session::Type::REGULAR, "REGULAR" );
    addItem( Session::Type::OBSERVING, "OBSERVING" );
    setDefault( Session::Type::REGULAR );
}

} // namespace caffa

std::shared_ptr<Session> Session::create( Type type, std::chrono::milliseconds timeout )
{
    return std::shared_ptr<Session>( new Session( type, timeout ) );
}

Session::Session( Type type, std::chrono::milliseconds timeout )
    : m_uuid( caffa::UuidGenerator::generate() )
    , m_type( type )
    , m_timeOut( timeout )
    , m_lastKeepAlive( std::chrono::steady_clock::now() )
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

void Session::setType( Type type )
{
    m_type = type;
}

bool Session::isExpired() const
{
    auto                                  now           = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastKeepAlive = m_lastKeepAlive;
    auto timeSinceKeepalive = std::chrono::duration_cast<std::chrono::milliseconds>( now - lastKeepAlive );

    bool isExpired = timeSinceKeepalive > m_timeOut;

    CAFFA_TRACE( "Was the session expired? " << isExpired );
    return isExpired;
}

void Session::updateKeepAlive() const
{
    CAFFA_TRACE( "Update keepalive for session " << m_uuid );
    m_lastKeepAlive = std::chrono::steady_clock::now();
}

Session::Type Session::typeFromUint( unsigned type )
{
    switch ( type )
    {
        case static_cast<unsigned>( Type::INVALID ):
            return Type::INVALID;
        case static_cast<unsigned>( Type::REGULAR ):
            return Type::REGULAR;
        case static_cast<unsigned>( Type::OBSERVING ):
            return Type::OBSERVING;
        default:
            break;
    }
    throw std::runtime_error( "Bad session type " + std::to_string( type ) );
}

std::chrono::milliseconds Session::timeout() const
{
    return m_timeOut;
}
