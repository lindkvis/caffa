//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cafObjectFactory.h"

#include "cafAssert.h"

#include <map>
#include <string>
#include <vector>

namespace caf
{
//==================================================================================================
/// "Private" class for implementation of a factory for ObjectBase derived objects
/// Every Object must register with this factory to be readable
/// This class can be considered private in the Pdm system
//==================================================================================================

class DefaultObjectFactory : public ObjectFactory
{
public:
    static DefaultObjectFactory* instance();

    template <typename ObjectBaseDerivative>
    bool registerCreator()
    {
        std::vector<std::string> classNameKeywords = ObjectBaseDerivative::classKeywordAliases();

        for ( std::string classNameKeyword : classNameKeywords )
        {
            auto entryIt = m_factoryMap.find( classNameKeyword );
            if ( entryIt != m_factoryMap.end() )
            {
                CAF_ASSERT( classNameKeyword != entryIt->first ); // classNameKeyword has already been used
                CAF_ASSERT( false ); // To be sure ..
                return false; // never hit;
            }
        }
        auto object = new ObjectCreator<ObjectBaseDerivative>();
        for ( std::string classNameKeyword : classNameKeywords )
        {
            m_factoryMap[classNameKeyword] = object;
        }
        return true;
    }

    std::vector<std::string> classKeywords() const override;

private:
    ObjectHandle* doCreate( const std::string& classNameKeyword, uint64_t ) override;

    DefaultObjectFactory() {}
    ~DefaultObjectFactory() override
    { /* Could clean up, but ... */
    }

    // Internal helper classes

    class ObjectCreatorBase
    {
    public:
        ObjectCreatorBase() {}
        virtual ~ObjectCreatorBase() {}
        virtual ObjectHandle* create() = 0;
    };

    template <typename ObjectBaseDerivative>
    class ObjectCreator : public ObjectCreatorBase
    {
    public:
        ObjectHandle* create() override { return new ObjectBaseDerivative(); }
    };

    // Map to store factory
    std::map<std::string, ObjectCreatorBase*> m_factoryMap;
};

} // End of namespace caf
