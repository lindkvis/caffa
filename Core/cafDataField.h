// ##################################################################################################
//
//    CAFFA
//    Copyright (C) 2024- Kontur AS
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

#include "cafDataFieldAccessor.h"
#include "cafFieldHandle.h"

namespace caffa
{
class Editor;
class Visitor;

class DataField : public FieldHandle
{
public:
    virtual void setUntypedAccessor( std::unique_ptr<DataFieldAccessorInterface> accessor ) = 0;

    void accept( Inspector* visitor ) const override;
    void accept( Editor* editor ) override;
};

} // namespace caffa