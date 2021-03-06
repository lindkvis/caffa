#include "cafObject.h"

#include "uuid.h"

#include <chrono>
#include <random>

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

    auto                         seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937                 generator( seed );
    uuids::uuid_random_generator gen( generator );
    m_uuid = uuids::to_string( gen() );
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
