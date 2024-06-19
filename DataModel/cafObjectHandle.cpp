// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2021- 3D-Radar AS
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

#include "cafObjectHandle.h"

#include "cafAssert.h"
#include "cafChildArrayField.h"
#include "cafFieldHandle.h"
#include "cafUuidGenerator.h"
#include "cafVisitor.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::ObjectHandle( bool generateUuid /* = true */ )
{
    if ( generateUuid )
    {
        m_uuid = UuidGenerator::generate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle::~ObjectHandle() noexcept
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<FieldHandle*> ObjectHandle::fields() const
{
    std::vector<FieldHandle*> fieldVector;
    for ( auto& [ignore, field] : m_fields )
    {
        fieldVector.push_back( field );
    }
    return fieldVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<MethodHandle*> ObjectHandle::methods() const
{
    std::vector<MethodHandle*> methodVector;
    for ( auto& [ignore, method] : m_methods )
    {
        methodVector.push_back( method );
    }
    return methodVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::addField( FieldHandle* field, const std::string& keyword )
{
    field->m_ownerObject = this;

    CAFFA_ASSERT( ObjectHandle::isValidKeyword( keyword ) );
    CAFFA_ASSERT( !keyword.empty() );
    CAFFA_ASSERT( !m_fields.contains( keyword ) && "Object already has a field with this keyword!" );

    field->setKeyword( keyword );
    m_fields[keyword] = field;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::addMethod( MethodHandle* method, const std::string& keyword )
{
    CAFFA_ASSERT( !keyword.empty() );
    CAFFA_ASSERT( !m_methods.contains( keyword ) && "Object already has a field with this keyword!" );

    CAFFA_ASSERT( ObjectHandle::isValidKeyword( keyword ) );
    method->setName( keyword );
    m_methods[keyword] = method;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ObjectHandle::findField( const std::string& keyword ) const
{
    auto it = m_fields.find( keyword );
    return it != m_fields.end() ? it->second : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MethodHandle* ObjectHandle::findMethod( const std::string& keyword ) const
{
    auto it = m_methods.find( keyword );
    return it != m_methods.end() ? it->second : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::accept( Inspector* visitor ) const
{
    visitor->visit( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::accept( Editor* editor )
{
    editor->visit( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& ObjectHandle::uuid() const
{
    return m_uuid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::setUuid( const std::string& uuid )
{
    m_uuid = uuid;
}
