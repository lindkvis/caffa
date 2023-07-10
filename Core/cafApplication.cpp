#include "cafApplication.h"
#include "cafAssert.h"

using namespace caffa;

Application* Application::s_instance = nullptr;

void caffa::to_json( nlohmann::json& jsonValue, const AppInfo& appInfo )
{
    jsonValue                  = nlohmann::json::object();
    jsonValue["name"]          = appInfo.name;
    jsonValue["type"]          = appInfo.appType;
    jsonValue["major_version"] = appInfo.majorVersion;
    jsonValue["minor_version"] = appInfo.minorVersion;
    jsonValue["patch_version"] = appInfo.patchVersion;
}

void caffa::from_json( const nlohmann::json& jsonValue, AppInfo& appInfo )
{
    appInfo.name         = jsonValue["name"].get<std::string>();
    appInfo.appType      = jsonValue["type"].get<unsigned>();
    appInfo.majorVersion = jsonValue["major_version"].get<int>();
    appInfo.minorVersion = jsonValue["minor_version"].get<int>();
    appInfo.patchVersion = jsonValue["patch_version"].get<int>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Application* Application::instance()
{
    return s_instance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Application::registerInstance( Application* instance )
{
    CAFFA_ASSERT( s_instance == nullptr );
    s_instance = instance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Application::Application( unsigned int capabilities )
    : m_capabilities( capabilities )
{
    registerInstance( const_cast<Application*>( this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::Application::Application( AppInfo::AppCapability capability )
    : m_capabilities( static_cast<unsigned int>( capability ) )
{
    registerInstance( const_cast<Application*>( this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Application::~Application()
{
    if ( s_instance )
    {
        s_instance = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Application::hasCapability( AppInfo::AppCapability typeToCheck ) const
{
    return ( m_capabilities & static_cast<unsigned int>( typeToCheck ) ) != 0u;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AppInfo Application::appInfo() const
{
    return { name(), majorVersion(), minorVersion(), patchVersion(), m_capabilities };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Application::assertCapability( AppInfo::AppCapability typeToAssert )
{
    auto app = instance();
    CAFFA_ASSERT( app->hasCapability( typeToAssert ) );
}