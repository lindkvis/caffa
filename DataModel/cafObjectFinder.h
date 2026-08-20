// ##################################################################################################
//
//    Copyright (C) 2023- Kontur AS
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
#include "cafChildFieldHandle.h"
#include "cafVisitor.h"

#include <functional>
#include <list>

namespace caffa
{
class ObjectHandle;

/**
 * A simple depth first object finder which exists as soon as it found a match
 */
template <typename ObjectType = ObjectHandle>
class ConstObjectFinder final : public Inspector
{
public:
    using Selector = std::function<bool( const ObjectType* )>;

    explicit ConstObjectFinder( Selector selector = nullptr )
        : m_selector( selector )
    {
    }

    void visit( const std::shared_ptr<const ObjectHandle>& object ) override
    {
        auto typedObject = std::dynamic_pointer_cast<ObjectType>( object );
        if ( typedObject && ( !m_selector || m_selector( typedObject ) ) )
        {
            m_object = typedObject;
            return;
        }

        for ( const auto field : object->fields() )
        {
            field->accept( this );
        }
    }

    void visit( const ChildFieldBaseHandle* field ) override
    {
        for ( const auto& object : field->childObjects() )
        {
            object->accept( this );
        }
    }

    void visit( const DataField* field ) override {}

    const std::shared_ptr<const ObjectType>& object() const { return m_object; }

private:
    Selector                          m_selector;
    std::shared_ptr<const ObjectType> m_object;
};

/**
 * A simple depth first object finder which exists as soon as it found a match
 */
template <typename ObjectType = ObjectHandle>
class ObjectFinder : public Editor
{
public:
    using Selector = std::function<bool( const ObjectType* )>;

    explicit ObjectFinder( Selector selector = nullptr )
        : m_selector( selector )
    {
    }

    void visit( const std::shared_ptr<ObjectHandle>& object ) override
    {
        auto typedObject = dynamic_cast<ObjectType*>( object.get() );
        if ( typedObject && ( !m_selector || m_selector( typedObject ) ) )
        {
            m_object = object;
            return;
        }

        for ( const auto field : object->fields() )
        {
            field->accept( this );
        }
    }

    void visit( ChildFieldBaseHandle* field ) override
    {
        if ( field->isReadable() )
        {
            for ( const auto& object : field->childObjects() )
            {
                object->accept( this );
            }
        }
    }

    void visit( DataField* field ) override {}

    const std::shared_ptr<ObjectType>& object() { return m_object; }

private:
    Selector                    m_selector;
    std::shared_ptr<ObjectType> m_object;
};
} // namespace caffa