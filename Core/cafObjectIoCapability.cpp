#include "cafObjectIoCapability.h"

#include "cafAssert.h"
#include "cafDefaultObjectFactory.h"
#include "cafFieldHandle.h"
#include "cafFieldJsonCapability.h"
#include "cafJsonSerializer.h"
#include "cafObjectHandle.h"
#include "cafStringTools.h"

#include <fstream>
#include <iostream>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectIoCapability::ObjectIoCapability( ObjectHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectIoCapability::~ObjectIoCapability() noexcept
{
}

//--------------------------------------------------------------------------------------------------
/// Check if a string is a valid element name
//
/// http://www.w3schools.com/xml/xml_elements.asp
///
/// JSON elements must follow these naming rules:
///   Names can contain letters, numbers, and other characters
///   Names cannot start with a number or punctuation character
///   Names cannot start with the letters xml (or JSON, or Xml, etc)
///   Names cannot contain spaces
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::isValidElementName( const std::string& name )
{
    if ( name.empty() )
    {
        return false;
    }

    if ( name.length() > 0 )
    {
        char firstChar = name[0];
        if ( std::isdigit( firstChar ) || firstChar == '.' )
        {
            return false;
        }
    }

    if ( name.size() >= 3 )
    {
        auto lower = caffa::StringTools::tolower( name );
        if ( lower.compare( 0, 3, "xml" ) == 0 )
        {
            return false;
        }
    }

    if ( name.find( ' ' ) != std::string::npos )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectIoCapability::copyBySerialization( ObjectFactory* objectFactory )
{
    return JsonSerializer( objectFactory ).copyBySerialization( m_owner );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> ObjectIoCapability::copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                                              ObjectFactory*     objectFactory )
{
    return JsonSerializer( objectFactory ).copyAndCastBySerialization( m_owner, destinationClassKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readFile( const std::string& fileName, IoType ioType /*=IoType::JSON*/ )
{
    std::ifstream inStream( fileName );
    if ( !inStream.good() )
    {
        CAFFA_ERROR( "Could not open file for reading: " << fileName );
        return false;
    }

    return readStream( inStream, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeFile( const std::string& fileName, IoType ioType /*=IoType::JSON*/ )
{
    std::ofstream outStream( fileName );
    if ( !outStream.good() )
    {
        CAFFA_ERROR( "Could not open file for writing: " << fileName );
        return false;
    }

    switch ( ioType )
    {
        case IoType::JSON:
        {
            // Do not write UUID or data types to file. UUID is only for dynamic connection to runtime objects.
            return writeStream( outStream,
                                JsonSerializer( DefaultObjectFactory::instance() )
                                    .setSerializeSchema( false )
                                    .setSerializeUuids( false ) );
        }
    }

    CAFFA_ERROR( "IO Type not implemented" );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readStream( std::istream& inStream, IoType ioType )
{
    switch ( ioType )
    {
        case IoType::JSON:
        {
            JsonSerializer jsonSerializer;
            return readStream( inStream, jsonSerializer );
        }
    }
    CAFFA_ERROR( "IO Type not implemented" );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeStream( std::ostream& outStream, IoType ioType )
{
    switch ( ioType )
    {
        case IoType::JSON:
        {
            JsonSerializer jsonSerializer;
            return writeStream( outStream, jsonSerializer );
        }
    }

    CAFFA_ERROR( "IO Type not implemented" );
    return false;
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

    std::vector<ObjectHandle*> children;
    for ( auto field : object->fields() )
    {
        if ( !field ) continue;

        auto fieldObjects = field->childObjects();
        children.insert( children.end(), fieldObjects.begin(), fieldObjects.end() );
    }

    for ( auto child : children )
    {
        initAfterReadRecursively( child );
    }

    auto ioCapability = object->capability<ObjectIoCapability>();
    if ( ioCapability )
    {
        ioCapability->initAfterRead();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::setupBeforeSaveRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;

    std::vector<ObjectHandle*> children;
    for ( auto field : object->fields() )
    {
        if ( !field ) continue;

        auto fieldObjects = field->childObjects();
        children.insert( children.end(), fieldObjects.begin(), fieldObjects.end() );
    }

    for ( auto child : children )
    {
        setupBeforeSaveRecursively( child );
    }

    auto ioCapability = object->capability<ObjectIoCapability>();
    if ( ioCapability )
    {
        ioCapability->setupBeforeSave();
    }
}

} // end namespace caffa
