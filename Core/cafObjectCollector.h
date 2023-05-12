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
class ConstObjectCollector : public Inspector
{
public:
    using Predicate = std::function<bool( const ObjectHandle* )>;

    ConstObjectCollector( Predicate predicate = nullptr )
        : m_predicate( predicate )
    {
    }

    void visitObject( const ObjectHandle* object )
    {
        auto typedObject = dynamic_cast<const ObjectType*>( object );
        if ( typedObject && ( !m_predicate || m_predicate( object ) ) )
        {
            m_objects.push_back( typedObject );
        }

        for ( auto field : object->fields() )
        {
            field->accept( this );
        }
    }

    void visitField( const FieldHandle* field ) override {}

    void visitChildField( const ChildFieldBaseHandle* childField ) override
    {
        for ( auto object : childField->childObjects() )
        {
            object->accept( this );
        }
    }

    const std::list<const ObjectType*>& objects() const { return m_objects; }

private:
    Predicate                    m_predicate;
    std::list<const ObjectType*> m_objects;
};

/**
 * A simple depth first collector
 */
template <typename ObjectType = ObjectHandle>
class ObjectCollector : public Editor
{
public:
    using Predicate = std::function<bool( const ObjectHandle* )>;

    ObjectCollector( Predicate predicate = nullptr )
        : m_predicate( predicate )
    {
    }

    void visitObject( ObjectHandle* object )
    {
        auto typedObject = dynamic_cast<ObjectType*>( object );
        if ( typedObject && ( !m_predicate || m_predicate( object ) ) )
        {
            m_objects.push_back( typedObject );
        }

        for ( auto field : object->fields() )
        {
            field->accept( this );
        }
    }

    void visitField( FieldHandle* field ) override {}

    void visitChildField( ChildFieldBaseHandle* childField ) override
    {
        for ( auto object : childField->childObjects() )
        {
            object->accept( this );
        }
    }

    const std::list<ObjectType*>& objects() { return m_objects; }

private:
    Predicate              m_predicate;
    std::list<ObjectType*> m_objects;
};
} // namespace caffa