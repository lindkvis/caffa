#include "cafObjectIoCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafFieldIoCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectJsonCapability.h"
#include "cafStringTools.h"

#include <fstream>
#include <iostream>

namespace caf
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
ObjectHandle* ObjectIoCapability::readUnknownObjectFromString( const std::string& string,
                                                               ObjectFactory*     objectFactory,
                                                               bool               isCopyOperation,
                                                               IoType             ioType /*= IoType::JSON */ )
{
    ObjectHandle* object = nullptr;

    switch ( ioType )
    {
        default:
            break;
        case IoType::JSON:
            object = ObjectJsonCapability::readUnknownObjectFromString( string, objectFactory, isCopyOperation );
            break;
        case IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::readObjectFromString( const std::string& string,
                                               ObjectFactory*     objectFactory,
                                               IoType             ioType /*= IoType::JSON */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoType::JSON:
            ObjectJsonCapability::readObjectFromString( m_owner, string, objectFactory );
            break;
        case IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ObjectIoCapability::writeObjectToString( IoType ioType /*= IoType::JSON */,
                                                     bool   writeServerAddress /*= false */ ) const
{
    std::string string;
    switch ( ioType )
    {
        default:
            break;
        case IoType::JSON:
            string = ObjectJsonCapability::writeObjectToString( m_owner, writeServerAddress );
            break;
        case IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return string;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ObjectHandle* ObjectIoCapability::copyBySerialization( ObjectFactory* objectFactory,
                                                            IoType         ioType /*= IoType::JSON */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoType::JSON:
            return ObjectJsonCapability::copyByJsonSerialization( m_owner, objectFactory );
            break;
        case IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ObjectHandle* ObjectIoCapability::copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                                   const std::string& sourceClassKeyword,
                                                                   ObjectFactory*     objectFactory,
                                                                   IoType             ioType /*= IoType::JSON */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoType::JSON:
            return ObjectJsonCapability::copyAndCastByJsonSerialization( m_owner,
                                                                         destinationClassKeyword,
                                                                         sourceClassKeyword,
                                                                         objectFactory );
            break;
        case IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return nullptr;
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
        auto lower = caf::StringTools::tolower( name );
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
void ObjectIoCapability::registerClassKeyword( const std::string& registerKeyword )
{
    m_classInheritanceStack.push_back( registerKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::inheritsClassWithKeyword( const std::string& testClassKeyword ) const
{
    return std::find( m_classInheritanceStack.begin(), m_classInheritanceStack.end(), testClassKeyword ) !=
           m_classInheritanceStack.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<std::string>& ObjectIoCapability::classInheritanceStack() const
{
    return m_classInheritanceStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readFile( const std::string& fileName, IoType ioType /*= IoType::JSON */ )
{
    std::ifstream inStream( fileName );
    if ( !inStream.good() ) return false;
    return readFile( inStream, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeFile( const std::string& fileName, IoType ioType /*= IoType::JSON */ )
{
    std::ofstream outStream( fileName );
    if ( !outStream.good() ) return false;
    return writeFile( outStream, ioType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::readFile( std::istream& stream, IoType ioType )
{
    try
    {
        switch ( ioType )
        {
            default:
                break;
            case IoType::JSON:
                ObjectJsonCapability::readFile( m_owner, stream );
                break;
            case IoType::SQL:
                CAF_ASSERT( "SQL writing is not implemented" );
                break;
        }
        initAfterReadRecursively();
    }
    catch ( ... )
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::writeFile( std::ostream& stream, IoType ioType, bool writeAddress )
{
    // Ask all objects to make them ready to write themselves to file
    setupBeforeSaveRecursively();

    try
    {
        switch ( ioType )
        {
            default:
                break;
            case IoType::JSON:
                ObjectJsonCapability::writeFile( m_owner, stream, writeAddress );
                break;
            case IoType::SQL:
                CAF_ASSERT( "SQL writing is not implemented" );
                break;
        }
    }
    catch ( ... )
    {
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
        if ( field ) field->childObjects( &children );
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
void ObjectIoCapability::resolveReferencesRecursively( ObjectHandle*              object,
                                                       std::vector<FieldHandle*>* fieldWithFailingResolve )
{
    if ( object == nullptr ) return;

    std::vector<FieldHandle*> fields = object->fields();

    std::vector<ObjectHandle*> children;
    size_t                     fIdx;
    for ( auto field : fields )
    {
        field->childObjects( &children );

        bool resolvedOk = field->capability<FieldIoCapability>()->resolveReferences();
        if ( fieldWithFailingResolve && !resolvedOk )
        {
            fieldWithFailingResolve->push_back( field );
        }
    }

    for ( auto child : children )
    {
        resolveReferencesRecursively( child, fieldWithFailingResolve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::resolveReferencesRecursively( std::vector<FieldHandle*>* fieldWithFailingResolve /*= nullptr*/ )
{
    std::vector<FieldHandle*> tempFields;
    resolveReferencesRecursively( this->m_owner, &tempFields );

    if ( fieldWithFailingResolve )
    {
        for ( auto f : tempFields )
        {
            fieldWithFailingResolve->push_back( f );
        }
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
        if ( field ) field->childObjects( &children );
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

} // end namespace caf
