#include "cafObject.h"

#include "cafUuidGenerator.h"

using namespace caffa;

CAFFA_ABSTRACT_SOURCE_INIT( Object, "Object" )

using namespace std::chrono;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::Object( bool generateUuid /* = false*/ )
    : ObjectHandle()
{
    addCapability( std::make_unique<caffa::ObjectIoCapability>( this ) );

    initField( m_uuid, "uuid" ).withScripting( "uuid", false, false );

    if ( generateUuid )
    {
        m_uuid = UuidGenerator::generate();
    }
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

std::unique_ptr<ObjectHandle> Object::deepClone( caffa::ObjectFactory* optionalObjectFactory ) const
{
    caffa::ObjectFactory* objectFactory = optionalObjectFactory ? optionalObjectFactory
                                                                : caffa::DefaultObjectFactory::instance();
    return capability<caffa::ObjectIoCapability>()->copyBySerialization( objectFactory );
}