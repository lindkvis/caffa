// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2022- 3D-Radar AS
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
#include "cafSerializer.h"

using namespace caffa;

Serializer::Serializer( ObjectFactory* objectFactory )
    : m_objectFactory( objectFactory )
    , m_writeTypesAndValidators( true )
    , m_serializeDataTypes( true )
    , m_serializeUuids( true )
{
}

Serializer& Serializer::setFieldSelector( FieldSelector fieldSelector )
{
    m_fieldSelector = fieldSelector;
    return *this;
}

Serializer& Serializer::setWriteTypesAndValidators( bool writeTypesAndValidators )
{
    m_writeTypesAndValidators = writeTypesAndValidators;
    return *this;
}

Serializer& Serializer::setSerializeDataTypes( bool serializeDataTypes )
{
    m_serializeDataTypes = serializeDataTypes;
    return *this;
}

Serializer& Serializer::setSerializeUuids( bool serializeUuids )
{
    m_serializeUuids = serializeUuids;
    return *this;
}

ObjectFactory* Serializer::objectFactory() const
{
    return m_objectFactory;
}

Serializer::FieldSelector Serializer::fieldSelector() const
{
    return m_fieldSelector;
}

bool Serializer::writeTypesAndValidators() const
{
    return m_writeTypesAndValidators;
}

bool Serializer::serializeDataTypes() const
{
    return m_serializeDataTypes;
}

bool Serializer::serializeUuids() const
{
    return m_serializeUuids;
}