#include "cafDefaultObjectFactory.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> DefaultObjectFactory::doCreate( const std::string_view& classKeyword )
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

} // End of namespace caffa
