#include "cafDefaultObjectFactory.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

std::list<std::string> DefaultObjectFactory::classes() const
{
    std::list<std::string> classList;
    for ( auto [name, creator] : m_factoryMap )
    {
        classList.push_back( name );
    }
    return classList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::Ptr DefaultObjectFactory::doCreate( const std::string_view& classKeyword )
{
    auto entryIt = m_factoryMap.find( classKeyword );
    if ( entryIt != m_factoryMap.end() )
    {
        return entryIt->second->create();
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
DefaultObjectFactory* DefaultObjectFactory::instance()
{
    static DefaultObjectFactory* fact = new DefaultObjectFactory;
    return fact;
}

void DefaultObjectFactory::doApplyAccessors( caffa::not_null<caffa::ObjectHandle*> )
{ // Do nothing. Fields already have a direct local accessor
}

} // End of namespace caffa
