#include "cafObjectIoCapability.h"

#include "cafAssert.h"
#include "cafFieldIoCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectJsonCapability.h"
#include "cafObjectXmlCapability.h"

#include "cafFieldHandle.h"

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
ObjectHandle*
    ObjectIoCapability::readUnknownObjectFromString( const QString&       string,
                                                        ObjectFactory*    objectFactory,
                                                        bool                 isCopyOperation,
                                                        IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    ObjectHandle* object = nullptr;

    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            object = ObjectXmlCapability::readUnknownObjectFromXmlString( string, objectFactory, isCopyOperation );
            break;
        case IoParameters::IoType::JSON:
            object = ObjectJsonCapability::readUnknownObjectFromString( string, objectFactory, isCopyOperation );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::readObjectFromString( const QString&       string,
                                                  ObjectFactory*    objectFactory,
                                                  IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            ObjectXmlCapability::readObjectFromXmlString( m_owner, string, objectFactory );
            break;
        case IoParameters::IoType::JSON:
            ObjectJsonCapability::readObjectFromString( m_owner, string, objectFactory );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString ObjectIoCapability::writeObjectToString( IoParameters::IoType ioType /*= IoParameters::IoType::XML */ ) const
{
    QString string;
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            string = ObjectXmlCapability::writeObjectToXmlString( m_owner );
            break;
        case IoParameters::IoType::JSON:
            string = ObjectJsonCapability::writeObjectToString( m_owner );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return string;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ObjectHandle*
    ObjectIoCapability::copyBySerialization( ObjectFactory*    objectFactory,
                                                IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            return ObjectXmlCapability::copyByXmlSerialization( m_owner, objectFactory );
        case IoParameters::IoType::JSON:
            return ObjectJsonCapability::copyByJsonSerialization( m_owner, objectFactory );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ObjectHandle*
    ObjectIoCapability::copyAndCastBySerialization( const QString&       destinationClassKeyword,
                                                       const QString&       sourceClassKeyword,
                                                       ObjectFactory*    objectFactory,
                                                       IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            return ObjectXmlCapability::copyAndCastByXmlSerialization( m_owner,
                                                                          destinationClassKeyword,
                                                                          sourceClassKeyword,
                                                                          objectFactory );
        case IoParameters::IoType::JSON:
            return ObjectJsonCapability::copyAndCastByJsonSerialization( m_owner,
                                                                            destinationClassKeyword,
                                                                            sourceClassKeyword,
                                                                            objectFactory );
            break;
        case IoParameters::IoType::SQL:
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
/// XML elements must follow these naming rules:
///   Names can contain letters, numbers, and other characters
///   Names cannot start with a number or punctuation character
///   Names cannot start with the letters xml (or XML, or Xml, etc)
///   Names cannot contain spaces
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::isValidElementName( const QString& name )
{
    if ( name.isEmpty() )
    {
        return false;
    }

    if ( name.size() > 0 )
    {
        QChar firstChar = name[0];
        if ( firstChar.isDigit() || firstChar == '.' )
        {
            return false;
        }
    }

    if ( name.size() >= 3 )
    {
        if ( name.left( 3 ).compare( "xml", Qt::CaseInsensitive ) == 0 )
        {
            return false;
        }
    }

    if ( name.contains( ' ' ) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::registerClassKeyword( const QString& registerKeyword )
{
    m_classInheritanceStack.push_back( registerKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectIoCapability::inheritsClassWithKeyword( const QString& testClassKeyword ) const
{
    return std::find( m_classInheritanceStack.begin(), m_classInheritanceStack.end(), testClassKeyword ) !=
           m_classInheritanceStack.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<QString>& ObjectIoCapability::classInheritanceStack() const
{
    return m_classInheritanceStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::readFile( const IoParameters& parameters )
{
    switch ( parameters.ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            ObjectXmlCapability::readFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::JSON:
            ObjectJsonCapability::readFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
    initAfterReadRecursively();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::writeFile( const IoParameters& parameters )
{
    // Ask all objects to make them ready to write themselves to file
    setupBeforeSaveRecursively();

    switch ( parameters.ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            ObjectXmlCapability::writeFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::JSON:
            ObjectJsonCapability::writeFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::initAfterReadRecursively( ObjectHandle* object )
{
    if ( object == nullptr ) return;

    std::vector<FieldHandle*> fields;
    object->fields( fields );

    std::vector<ObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] ) fields[fIdx]->childObjects( &children );
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        initAfterReadRecursively( children[cIdx] );
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

    std::vector<FieldHandle*> fields;
    object->fields( fields );

    std::vector<ObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        FieldHandle* field = fields[fIdx];
        if ( field )
        {
            field->childObjects( &children );

            bool resolvedOk = field->capability<FieldIoCapability>()->resolveReferences();
            if ( fieldWithFailingResolve && !resolvedOk )
            {
                fieldWithFailingResolve->push_back( field );
            }
        }
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        resolveReferencesRecursively( children[cIdx], fieldWithFailingResolve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectIoCapability::resolveReferencesRecursively(
    std::vector<FieldHandle*>* fieldWithFailingResolve /*= nullptr*/ )
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

    std::vector<FieldHandle*> fields;
    object->fields( fields );

    std::vector<ObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] ) fields[fIdx]->childObjects( &children );
    }

    size_t cIdx;
    for ( cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        setupBeforeSaveRecursively( children[cIdx] );
    }

    auto ioCapability = object->capability<ObjectIoCapability>();
    if ( ioCapability )
    {
        ioCapability->setupBeforeSave();
    }
}

} // end namespace caf
