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
#include "cafFieldHandle.h"
#include "cafUuidGenerator.h"
#include "cafVisitor.h"

#include <ranges>

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
ObjectHandle::~ObjectHandle() noexcept = default;

bool ObjectHandle::isValidKeyword( const std::string& type )
{
    if ( type == "keyword" ) return false;
    if ( type == "uuid" ) return false;

    if ( StringTools::isdigit( type[0] ) ) return false;

    const auto end = std::ranges::find( type, '\0' );

    const auto validCount = std::count_if( type.cbegin(), end, ObjectHandle::isValidCharacter );
    const auto invalidCount =
        std::count_if( type.cbegin(), end, []( auto c ) { return !ObjectHandle::isValidCharacter( c ); } );

    return validCount > 0u && invalidCount == 0u;
}

bool ObjectHandle::matchesClassKeyword( const std::string_view&         classKeyword,
                                        const std::vector<std::string>& inheritanceStack )
{
    return std::any_of( inheritanceStack.begin(),
                        inheritanceStack.end(),
                        [&classKeyword]( const std::string& testKeyword ) { return classKeyword == testKeyword; } );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<FieldHandle*> ObjectHandle::fields() const
{
    std::vector<FieldHandle*> fieldVector;
    for ( auto& field : std::views::values( m_fields ) )
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
    for ( auto& method : std::views::values( m_methods ) )
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
    visitor->visit( shared_from_this() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectHandle::accept( Editor* editor )
{
    editor->visit( shared_from_this() );
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
