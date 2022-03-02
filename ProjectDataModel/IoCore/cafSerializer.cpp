//##################################################################################################
//
//   Caffa
//   Copyright (C) 2022- 3D-Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
#include "cafSerializer.h"

using namespace caffa;

Serializer::Serializer( bool copyDataValues, ObjectFactory* objectFactory, FieldSelector fieldSelector, bool writeUuids )
    : m_copyDataValues( copyDataValues )
    , m_objectFactory( objectFactory )
    , m_fieldSelector( fieldSelector )
    , m_writeUuids( writeUuids )
{
}

bool Serializer::copyDataValues() const
{
    return m_copyDataValues;
}

Serializer::FieldSelector Serializer::fieldSelector() const
{
    return m_fieldSelector;
}

ObjectFactory* Serializer::objectFactory() const
{
    return m_objectFactory;
}

bool Serializer::writeUuids() const
{
    return m_writeUuids;
}