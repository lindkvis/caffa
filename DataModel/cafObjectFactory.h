// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011- Ceetron AS (Changes up until April 2021)
//    Copyright (C) 2021- Kontur AS (Changes from April 2021 and onwards)
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

#include "cafObjectHandle.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace caffa
{
//==================================================================================================
//
// Factory interface for creating CAF objects derived from ObjectHandle based on class name keyword
//
//==================================================================================================
class ObjectFactory
{
public:
    ObjectHandle::Ptr create( const std::string_view& classKeyword ) { return doCreate( classKeyword ); }

    virtual std::string name() const = 0;

protected:
    ObjectFactory() {}
    virtual ~ObjectFactory() {}

private:
    virtual ObjectHandle::Ptr doCreate( const std::string_view& classKeyword ) = 0;
};

} // End of namespace caffa
