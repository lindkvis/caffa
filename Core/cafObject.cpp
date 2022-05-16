#include "cafObject.h"

#include "uuid.h"

#include <chrono>
#include <random>

using namespace caffa;

CAFFA_ABSTRACT_SOURCE_INIT( Object, "Object" )

using namespace std::chrono;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
{
    auto     now                     = system_clock::now();
    auto     nanoseconds_since_epoch = now.time_since_epoch();
    auto     seconds_since_epoch     = duration_cast<seconds>( nanoseconds_since_epoch );
    unsigned nanoseconds_since_last_second =
        static_cast<unsigned>( ( nanoseconds_since_epoch - duration_cast<nanoseconds>( seconds_since_epoch ) ).count() );

    std::mt19937                 generator( nanoseconds_since_last_second );
    uuids::uuid_random_generator gen( generator );

    initField( m_uuid, "UUID" ).withScripting( "UUID", true, false ).withDefault( uuids::to_string( gen() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::~Object() noexcept
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Object::classKeywordDynamic() const
{
    return this->classKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string Object::uuid() const
{
    return m_uuid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::setUuid( const std::string& uuid )
{
    m_uuid = uuid;
}
