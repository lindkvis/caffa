#include "cafObject.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace caffa;

CAFFA_ABSTRACT_SOURCE_INIT( Object, "Object" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
    , ObjectUiCapability( this, false )
{
    initField( m_uuid, "uuid" ).withScripting();
    boost::uuids::random_generator generator;
    m_uuid = boost::uuids::to_string( generator() );
}

void Object::assignUiInfo( const std::string& uiName,
                           const std::string& iconResourceName,
                           const std::string& toolTip,
                           const std::string& whatsThis )
{
    std::string       validUiName = uiName.empty() ? classKeyword() : uiName;
    caffa::UiItemInfo objDescr( validUiName, iconResourceName, toolTip, whatsThis );
    this->setUiItemInfo( objDescr );
}

std::string Object::uuid() const
{
    return m_uuid;
}

void Object::setUuid( const std::string& uuid )
{
    m_uuid = uuid;
}
