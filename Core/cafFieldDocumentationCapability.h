//##################################################################################################
//
//   Caffa
//   Copyright (C) 2022- Kontur AS
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

#include "cafFieldCapability.h"

#include <string>

namespace caffa
{
class FieldHandle;

class FieldDocumentationCapability : public FieldCapability
{
public:
    FieldDocumentationCapability( const std::string& documentation );

    std::string documentation() const;

protected:
    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* field ) override;

private:
    std::string m_documentation;

    FieldHandle* m_owner;
};

} // namespace caffa
