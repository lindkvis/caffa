#include "cafObject.h"

#include "uuid.h"

#include <chrono>
#include <random>

using namespace caffa;

CAFFA_ABSTRACT_SOURCE_INIT( Object, "Object" )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
{
    auto                         seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937                 generator( seed );
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
