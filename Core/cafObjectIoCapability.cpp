#include "cafObjectIoCapability.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafFieldHandle.h"
#include "cafFieldJsonCapability.h"
#include "cafJsonSerializer.h"
#include "cafObjectCollector.h"
#include "cafObjectHandle.h"

#include <fstream>
#include <iostream>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectIoCapability::ObjectIoCapability( ObjectHandle* owner )
{
    m_owner = owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectIoCapability::~ObjectIoCapability() noexcept
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::initAfterReadRecursively()
{
    initAfterReadRecursively( m_owner );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::setupBeforeSaveRecursively()
{
    setupBeforeSaveRecursively( m_owner );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr ObjectIoCapability::copyBySerialization( ObjectFactory* objectFactory ) const
{
    return JsonSerializer( objectFactory ).setSerializeUuids( false ).copyBySerialization( m_owner );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr ObjectIoCapability::copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                                  ObjectFactory*     objectFactory ) const
{
    return JsonSerializer( objectFactory ).setSerializeUuids( false ).copyAndCastBySerialization( m_owner, destinationClassKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readFile( const std::string& fileName )
{
    std::ifstream inStream( fileName );
    if ( !inStream.good() )
    {
        CAFFA_ERROR( "Could not open file for reading: " << fileName );
        return false;
    }

    return readStream( inStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeFile( const std::string& fileName )
{
    std::ofstream outStream( fileName );
    if ( !outStream.good() )
    {
        CAFFA_ERROR( "Could not open file for writing: " << fileName );
        return false;
    }
    // Do not write UUID or data types to file. UUID is only for dynamic connection to runtime objects.
    return writeStream( outStream,
                        JsonSerializer( DefaultObjectFactory::instance() ).setSerializeSchema( false ).setSerializeUuids( false ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readStream( std::istream& inStream )
{
    JsonSerializer jsonSerializer;
    return readStream( inStream, jsonSerializer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeStream( std::ostream& outStream )
{
    JsonSerializer jsonSerializer;
    return writeStream( outStream, jsonSerializer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readStream( std::istream& inStream, const Serializer& serializer )
{
    try
    {
        serializer.readStream( m_owner, inStream );
        this->initAfterReadRecursively();
    }
    catch ( std::runtime_error& err )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeStream( std::ostream& outStream, const Serializer& serializer )
{
    try
    {
        this->setupBeforeSaveRecursively();
        serializer.writeStream( m_owner, outStream );
    }
    catch ( std::runtime_error& err )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::initAfterReadRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;

    ObjectCollector collector;
    object->accept( &collector );

    for ( auto child : collector.objects() )
    {
        child->initAfterRead();
    }

    object->initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::setupBeforeSaveRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;

    ObjectCollector collector;
    object->accept( &collector );

    for ( auto child : collector.objects() )
    {
        child->setupBeforeSave();
    }

    object->setupBeforeSave();
}

} // end namespace caffa
