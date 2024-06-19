// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011- Ceetron AS (Changes up until April 2021)
//    Copyright (C) 2021- Kontur AS (Changes from April 2021 and onwards)
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################

#pragma once

#include "cafObjectFactory.h"

#include "cafAssert.h"

#include <map>
#include <string>
#include <vector>

namespace caffa
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

    std::string name() const override { return "Default ObjectFactory"; }

    std::list<std::string> classes() const;

    template <typename ObjectBaseDerivative>
    bool registerCreator()
    {
        auto classKeyword = ObjectBaseDerivative::classKeywordStatic();

        auto entryIt = m_factoryMap.find( classKeyword );
        if ( entryIt != m_factoryMap.end() )
        {
            CAFFA_ASSERT( classKeyword != entryIt->first ); // classKeyword has already been used
            CAFFA_ASSERT( false ); // To be sure ..
            return false; // never hit;
        }
        auto object                               = new ObjectCreator<ObjectBaseDerivative>();
        m_factoryMap[std::string( classKeyword )] = object;
        return true;
    }

private:
    std::shared_ptr<ObjectHandle> doCreate( const std::string_view& classKeyword ) override;

    DefaultObjectFactory() {}
    ~DefaultObjectFactory() override { /* Could clean up, but ... */ }

    // Internal helper classes

    class ObjectCreatorBase
    {
    public:
        ObjectCreatorBase() {}
        virtual ~ObjectCreatorBase() {}
        virtual std::shared_ptr<ObjectHandle> create() = 0;
    };

    template <typename ObjectBaseDerivative>
    class ObjectCreator : public ObjectCreatorBase
    {
    public:
        std::shared_ptr<ObjectHandle> create() override { return std::make_shared<ObjectBaseDerivative>(); }
    };

    // Map to store factory
    std::map<std::string, ObjectCreatorBase*, std::less<>> m_factoryMap;
};

} // End of namespace caffa
