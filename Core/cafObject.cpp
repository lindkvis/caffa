#include "cafObject.h"

#include "cafUuidGenerator.h"

using namespace caffa;

using namespace std::chrono;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::Object( bool generateUuid /* = false*/ )
    : ObjectHandle()
{
    addCapability( std::make_unique<caffa::ObjectIoCapability>( this ) );

    initField( m_uuid, "uuid" );

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