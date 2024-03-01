#include "cafObject.h"

#include "cafLogger.h"
#include "cafObjectPerformer.h"
#include "cafUuidGenerator.h"

#include <fstream>

using namespace caffa;

using namespace std::chrono;

CAFFA_SOURCE_INIT( Object );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object::Object( bool generateUuid /* = true*/ )
    : ObjectHandle()
{
    initField( m_uuid, "uuid" ).withScripting( true, false );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr Object::deepClone( caffa::ObjectFactory* optionalObjectFactory ) const
{
    caffa::ObjectFactory* objectFactory = optionalObjectFactory ? optionalObjectFactory
                                                                : caffa::DefaultObjectFactory::instance();

    return JsonSerializer( objectFactory ).setSerializeUuids( false ).copyBySerialization( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Object::readFromJsonFile( const std::string& filePath )
{
    std::ifstream inStream( filePath );
    if ( !inStream.good() )
    {
        CAFFA_ERROR( "Could not open file for reading: " << filePath );
        return false;
    }

    JsonSerializer serializer;

    try
    {
        serializer.readStream( this, inStream );

        ObjectPerformer<> performer( []( ObjectHandle* object ) { object->initAfterRead(); } );
        performer.visit( this );
    }
    catch ( const std::exception& err )
    {
        CAFFA_ERROR( err.what() );
        return false;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Generic object reading error" );
        return false;
    }
    return true;
}

bool Object::writeToJsonFile( const std::string& filePath ) const
{
    std::ofstream outStream( filePath );

    if ( !outStream.good() )
    {
        CAFFA_ERROR( "Could not open file for writing: " << filePath );
        return false;
    }

    try
    {
        JsonSerializer serializer;
        serializer.setSerializeUuids( false );
        serializer.writeStream( this, outStream );
    }
    catch ( const std::exception& err )
    {
        CAFFA_ERROR( err.what() );
        return false;
    }
    catch ( ... )
    {
        CAFFA_ERROR( "Generic object writing error" );
        return false;
    }
    return true;
}