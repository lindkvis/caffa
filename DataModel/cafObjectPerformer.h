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
#include "cafObjectHandle.h"
#include "cafVisitor.h"

namespace caffa
{
/**
 * A simple depth first Editor that performs tasks on objects found
 */
template <typename ObjectType = ObjectHandle>
class ObjectPerformer final : public Editor
{
public:
    using Callback = std::function<void( ObjectType* )>;
    using Selector = std::function<bool( const ObjectType* )>;

    explicit ObjectPerformer( Callback callback, Selector selector = nullptr )
        : m_selector( selector )
        , m_callback( callback )
    {
    }

    void visit( const std::shared_ptr<ObjectHandle>& object ) override
    {
        if ( !object ) return;

        auto typedObject = dynamic_cast<ObjectType*>( object.get() );
        if ( typedObject && ( !m_selector || m_selector( typedObject ) ) )
        {
            m_callback( typedObject );
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
    Selector m_selector;
    Callback m_callback;
};
} // namespace caffa