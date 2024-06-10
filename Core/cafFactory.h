// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Changes since 2024:
//    Copyright (C) 2024- Kontur AS
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

#include "cafAssert.h"

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

// Taken from gtest.h
//
// Due to C++ preprocessor weirdness, we need double indirection to
// concatenate two tokens when one of them is __LINE__.  Writing
//
//   foo ## __LINE__
//
// will result in the token foo__LINE__, instead of foo followed by
// the current line number.  For more details, see
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.6
#define CAFFA_FACTORY_CONCATENATE_STRINGS( foo, bar ) CAFFA_FACTORY_CONCATENATE_STRINGS_IMPL_( foo, bar )
#define CAFFA_FACTORY_CONCATENATE_STRINGS_IMPL_( foo, bar ) foo##bar

#define CAFFA_UNIQUE_COMPILE_UNIT_VAR_NAME( foo ) CAFFA_FACTORY_CONCATENATE_STRINGS( foo, __LINE__ )

/// Macros to simplify registering entries in a factory
/// There are two, to make it possible to use two registrations in one macro

#define CAFFA_FACTORY_REGISTER( BaseType, TypeToCreate, KeyType, key )   \
    static bool CAFFA_UNIQUE_COMPILE_UNIT_VAR_NAME( my##TypeToCreate ) = \
        caffa::Factory<BaseType, KeyType>::instance() -> registerCreator<TypeToCreate>( key )
#define CAFFA_FACTORY_REGISTER2( BaseType, TypeToCreate, KeyType, key )   \
    static bool CAFFA_UNIQUE_COMPILE_UNIT_VAR_NAME( my2##TypeToCreate ) = \
        caffa::Factory<BaseType, KeyType>::instance() -> registerCreator<TypeToCreate>( key )

namespace caffa
{
//==================================================================================================
/// A generic Factory class template
/// Usage:
///     Simply add the classes that is supposed to be created by the factory by doing the folowing:
///
///     caffa::Factory<BaseType, KeyType>::instance()->registerCreator<TypeToCreate>(key);
///
///     This must only be done once for each TypeToCreate. It will assert if you try to do it several times.
///     This method returns a bool to make it possible to make this initialization as a static variable initialization.
///     That is useful if you do not want a centralized registering (but rather making each class register itself):
///
///     static bool uniqueVarname = caffa::Factory<BaseType, KeyType>::instance()->registerCreator<TypeToCreate>(key);
///
///     You can also use the macro CAFFA_FACTORY_REGISTER(BaseType, TypeToCreate, KeyType, key)
///
///     See also cafUiFieldEditorHandle.h for an advanced example.
///
///     When you need an object:
///
///     BaseType* newObject = caffa::Factory<BaseType, KeyType>::instance()->create(key);
//==================================================================================================

template <typename BaseType, typename KeyType>
class Factory
{
    class ObjectCreatorBase;

public:
    static Factory<BaseType, KeyType>* instance()
    {
        static Factory<BaseType, KeyType>* fact = new Factory<BaseType, KeyType>;
        return fact;
    }

    template <typename TypeToCreate>
    bool registerCreator( const KeyType& key )
    {
        auto entryIt = m_factoryMap.find( key );
        if ( entryIt == m_factoryMap.end() )
        {
            m_factoryMap[key] = std::make_unique<ObjectCreator<TypeToCreate>>();
            return true;
        }
        return false;
    }

    std::shared_ptr<BaseType> create( const KeyType& key )
    {
        auto entryIt = m_factoryMap.find( key );
        if ( entryIt != m_factoryMap.end() )
        {
            return entryIt->second->create();
        }
        else
        {
            return nullptr;
        }
    }

    std::vector<KeyType> allKeys()
    {
        std::vector<KeyType> keys;

        for ( auto entryIt = m_factoryMap.begin(); entryIt != m_factoryMap.end(); ++entryIt )
        {
            keys.push_back( entryIt->first );
        }

        return keys;
    }

private:
    Factory() {}
    ~Factory() = default;

    // Internal helper classes

    class ObjectCreatorBase
    {
    public:
        ObjectCreatorBase() {}
        virtual ~ObjectCreatorBase() {}
        virtual std::shared_ptr<BaseType> create() = 0;
    };

    template <typename TypeToCreate>
    class ObjectCreator : public ObjectCreatorBase
    {
    public:
        std::shared_ptr<BaseType> create() override { return std::make_shared<TypeToCreate>(); }
    };

    // Map to store factory
    std::map<KeyType, std::unique_ptr<ObjectCreatorBase>> m_factoryMap;
};

} // End of namespace caffa
