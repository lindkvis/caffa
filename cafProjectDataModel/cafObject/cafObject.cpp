#include "cafObject.h"

using namespace caffa;

CAF_ABSTRACT_SOURCE_INIT( Object, "Object" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
    , ObjectUiCapability( this, false )
{
}

void Object::assignUiInfo( const std::string& uiName,
                           const std::string& iconResourceName,
                           const std::string& toolTip,
                           const std::string& whatsThis )
{
    std::string     validUiName = uiName.empty() ? classKeyword() : uiName;
    caffa::UiItemInfo objDescr( validUiName, iconResourceName, toolTip, whatsThis );
    this->setUiItemInfo( objDescr );
}
