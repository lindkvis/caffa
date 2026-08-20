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

#include "cafChildFieldHandle.h"
#include "cafVisitor.h"

#include <list>

namespace caffa
{
class ObjectHandle;

/**
 * A simple depth first collector of const pointers to objects
 */
template <typename ObjectType = ObjectHandle>
class ConstObjectCollector final : public Inspector
{
public:
    using Selector = std::function<bool( const ObjectType* )>;

    explicit ConstObjectCollector( Selector selector = nullptr )
        : m_selector( selector )
    {
    }

    void visit( const std::shared_ptr<const ObjectHandle>& object ) override
    {
        if ( !object ) return;

        auto typedObject = std::dynamic_pointer_cast<const ObjectType>( object );
        if ( typedObject && ( !m_selector || m_selector( typedObject.get() ) ) )
        {
            m_objects.push_back( typedObject );
        }

        for ( const auto field : object->fields() )
        {
            if ( field->isReadable() )
            {
                field->accept( this );
            }
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

    const std::list<std::shared_ptr<const ObjectType>>& objects() const { return m_objects; }

private:
    Selector                                     m_selector;
    std::list<std::shared_ptr<const ObjectType>> m_objects;
};

/**
 * A simple depth first collector
 */
template <typename ObjectType = ObjectHandle>
class ObjectCollector final : public Editor
{
public:
    using Selector = std::function<bool( const ObjectType* )>;

    explicit ObjectCollector( Selector selector = nullptr )
        : m_selector( selector )
    {
    }
    const std::list<std::shared_ptr<ObjectType>>& objects() { return m_objects; }

    void visit( const std::shared_ptr<ObjectHandle>& object ) override
    {
        auto typedObject = std::dynamic_pointer_cast<ObjectType>( object );
        if ( typedObject && ( !m_selector || m_selector( typedObject.get() ) ) )
        {
            m_objects.push_back( typedObject );
        }

        for ( const auto field : object->fields() )
        {
            if ( field->isWritable() )
            {
                field->accept( this );
            }
        }
    }

    void visit( ChildFieldBaseHandle* field ) override
    {
        for ( const auto& object : field->childObjects() )
        {
            object->accept( this );
        }
    }

    void visit( DataField* field ) override {}

private:
    Selector                               m_selector;
    std::list<std::shared_ptr<ObjectType>> m_objects;
};
} // namespace caffa