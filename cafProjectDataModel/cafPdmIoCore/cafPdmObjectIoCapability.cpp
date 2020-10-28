#include "cafPdmObjectIoCapability.h"

#include "cafAssert.h"
#include "cafPdmFieldIoCapability.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectJsonCapability.h"
#include "cafPdmObjectXmlCapability.h"

#include "cafPdmFieldHandle.h"

#include <iostream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectIoCapability::PdmObjectIoCapability( PdmObjectHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle*
    PdmObjectIoCapability::readUnknownObjectFromString( const QString&       string,
                                                        PdmObjectFactory*    objectFactory,
                                                        bool                 isCopyOperation,
                                                        IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    PdmObjectHandle* object = nullptr;

    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            object = PdmObjectXmlCapability::readUnknownObjectFromXmlString( string, objectFactory, isCopyOperation );
            break;
        case IoParameters::IoType::JSON:
            object = PdmObjectJsonCapability::readUnknownObjectFromString( string, objectFactory, isCopyOperation );
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
void PdmObjectIoCapability::readObjectFromString( const QString&       string,
                                                  PdmObjectFactory*    objectFactory,
                                                  IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            PdmObjectXmlCapability::readObjectFromXmlString( m_owner, string, objectFactory );
            break;
        case IoParameters::IoType::JSON:
            PdmObjectJsonCapability::readObjectFromString( m_owner, string, objectFactory );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectIoCapability::writeObjectToString( IoParameters::IoType ioType /*= IoParameters::IoType::XML */ ) const
{
    QString string;
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            string = PdmObjectXmlCapability::writeObjectToXmlString( m_owner );
            break;
        case IoParameters::IoType::JSON:
            string = PdmObjectJsonCapability::writeObjectToString( m_owner );
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
caf::PdmObjectHandle*
    PdmObjectIoCapability::copyBySerialization( PdmObjectFactory*    objectFactory,
                                                IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            return PdmObjectXmlCapability::copyByXmlSerialization( m_owner, objectFactory );
        case IoParameters::IoType::JSON:
            return PdmObjectJsonCapability::copyByJsonSerialization( m_owner, objectFactory );
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
caf::PdmObjectHandle*
    PdmObjectIoCapability::copyAndCastBySerialization( const QString&       destinationClassKeyword,
                                                       const QString&       sourceClassKeyword,
                                                       PdmObjectFactory*    objectFactory,
                                                       IoParameters::IoType ioType /*= IoParameters::IoType::XML */ )
{
    switch ( ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            return PdmObjectXmlCapability::copyAndCastByXmlSerialization( m_owner,
                                                                          destinationClassKeyword,
                                                                          sourceClassKeyword,
                                                                          objectFactory );
        case IoParameters::IoType::JSON:
            return PdmObjectJsonCapability::copyAndCastByJsonSerialization( m_owner,
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
bool PdmObjectIoCapability::isValidElementName( const QString& name )
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
void PdmObjectIoCapability::registerClassKeyword( const QString& registerKeyword )
{
    m_classInheritanceStack.push_back( registerKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmObjectIoCapability::inheritsClassWithKeyword( const QString& testClassKeyword ) const
{
    return std::find( m_classInheritanceStack.begin(), m_classInheritanceStack.end(), testClassKeyword ) !=
           m_classInheritanceStack.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<QString>& PdmObjectIoCapability::classInheritanceStack() const
{
    return m_classInheritanceStack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectIoCapability::readFile( const IoParameters& parameters )
{
    switch ( parameters.ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            PdmObjectXmlCapability::readFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::JSON:
            PdmObjectJsonCapability::readFile( m_owner, parameters.ioDevice );
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
void PdmObjectIoCapability::writeFile( const IoParameters& parameters )
{
    // Ask all objects to make them ready to write themselves to file
    setupBeforeSaveRecursively();

    switch ( parameters.ioType )
    {
        default:
            break;
        case IoParameters::IoType::XML:
            PdmObjectXmlCapability::writeFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::JSON:
            PdmObjectJsonCapability::writeFile( m_owner, parameters.ioDevice );
            break;
        case IoParameters::IoType::SQL:
            CAF_ASSERT( "SQL writing is not implemented" );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectIoCapability::initAfterReadRecursively( PdmObjectHandle* object )
{
    if ( object == nullptr ) return;

    std::vector<PdmFieldHandle*> fields;
    object->fields( fields );

    std::vector<PdmObjectHandle*> children;
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

    auto ioCapability = object->capability<PdmObjectIoCapability>();
    if ( ioCapability )
    {
        ioCapability->initAfterRead();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectIoCapability::resolveReferencesRecursively( PdmObjectHandle*              object,
                                                          std::vector<PdmFieldHandle*>* fieldWithFailingResolve )
{
    if ( object == nullptr ) return;

    std::vector<PdmFieldHandle*> fields;
    object->fields( fields );

    std::vector<PdmObjectHandle*> children;
    size_t                        fIdx;
    for ( fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        PdmFieldHandle* field = fields[fIdx];
        if ( field )
        {
            field->childObjects( &children );

            bool resolvedOk = field->capability<PdmFieldIoCapability>()->resolveReferences();
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
void PdmObjectIoCapability::resolveReferencesRecursively(
    std::vector<PdmFieldHandle*>* fieldWithFailingResolve /*= nullptr*/ )
{
    std::vector<PdmFieldHandle*> tempFields;
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
void PdmObjectIoCapability::setupBeforeSaveRecursively( PdmObjectHandle* object )
{
    if ( object == nullptr ) return;

    std::vector<PdmFieldHandle*> fields;
    object->fields( fields );

    std::vector<PdmObjectHandle*> children;
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

    auto ioCapability = object->capability<PdmObjectIoCapability>();
    if ( ioCapability )
    {
        ioCapability->setupBeforeSave();
    }
}

} // end namespace caf
