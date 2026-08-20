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
#include <memory>
#include <ranges>
#include <string>

namespace caffa
{
//==================================================================================================
/// "Private" class for implementation of a factory for ObjectBase derived objects
/// Every Object must register with this factory to be readable
/// This class can be considered private in the Pdm system
//==================================================================================================

class DefaultObjectFactory final : public ObjectFactory
{
public:
    static std::shared_ptr<DefaultObjectFactory> instance();
    ~DefaultObjectFactory() override = default;

    [[nodiscard]] std::string name() const override { return "Default ObjectFactory"; }

    [[nodiscard]] std::ranges::view auto classes() const { return std::views::keys( m_factoryMap ); }

    template <typename ObjectBaseDerivative>
    bool registerCreator()
    {
        auto classKeyword = ObjectBaseDerivative::classKeywordStatic();

        if ( auto entryIt = m_factoryMap.find( classKeyword ); entryIt != m_factoryMap.end() )
        {
            CAFFA_ASSERT( classKeyword != entryIt->first ); // classKeyword has already been used
            CAFFA_ASSERT( false ); // To be sure ..
            return false; // never hit;
        }
        auto object                               = std::make_unique<ObjectCreator<ObjectBaseDerivative>>();
        m_factoryMap[std::string( classKeyword )] = std::move( object );
        return true;
    }

private:
    std::shared_ptr<ObjectHandle> doCreate( const std::string_view& classKeyword ) override;

    DefaultObjectFactory() = default;

    // Internal helper classes

    class ObjectCreatorBase
    {
    public:
        ObjectCreatorBase()                            = default;
        virtual ~ObjectCreatorBase()                   = default;
        virtual std::shared_ptr<ObjectHandle> create() = 0;
    };

    template <typename ObjectBaseDerivative>
    class ObjectCreator final : public ObjectCreatorBase
    {
    public:
        std::shared_ptr<ObjectHandle> create() override { return std::make_shared<ObjectBaseDerivative>(); }
    };

    // Map to store factory
    std::map<std::string, std::shared_ptr<ObjectCreatorBase>, std::less<>> m_factoryMap;
};

} // End of namespace caffa
