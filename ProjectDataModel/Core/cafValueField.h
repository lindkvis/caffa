//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
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
//##################################################################################################
#pragma once

#include "cafDataFieldAccessor.h"
#include "cafFieldHandle.h"
#include "cafPortableDataType.h"

#include <any>
#include <typeinfo>

namespace caffa
{
// Type traits magic to check if a template argument is a vector
template <typename T>
struct is_vector : public std::false_type
{
};
template <typename T, typename A>
struct is_vector<std::vector<T, A>> : public std::true_type
{
};

class ValueField : public FieldHandle
{
public:
    virtual Variant toVariant() const                        = 0;
    virtual void    setFromVariant( const Variant& variant ) = 0;
    virtual bool    isReadOnly() const                       = 0;
};

template <typename DataType>
class TypedValueField : public ValueField
{
public:
    using FieldDataType = DataType;

public:
    virtual DataType value() const                          = 0;
    virtual void     setValue( const DataType& fieldValue ) = 0;

    // TODO: replace with portable solution with template overrides
    std::string dataType() const override { return PortableDataType<DataType>::name(); }
};

} // End of namespace caffa
