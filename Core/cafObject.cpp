#include "cafObject.h"

#include "cafUuidGenerator.h"

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
    initField( m_uuid, "uuid" ).withScripting( "uuid", false, false );
    m_uuid = UuidGenerator::generate();
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
