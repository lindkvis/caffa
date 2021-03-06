#include "cafDefaultObjectFactory.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///  ObjectFactory implementations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> DefaultObjectFactory::doCreate( const std::string& classNameKeyword )
{
    std::map<std::string, ObjectCreatorBase*>::iterator entryIt;
    entryIt = m_factoryMap.find( classNameKeyword );
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
std::vector<std::string> DefaultObjectFactory::classKeywords() const
{
    std::vector<std::string> names;

    for ( const auto& entry : m_factoryMap )
    {
        names.push_back( entry.first );
    }

    return names;
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
