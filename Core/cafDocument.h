// ##################################################################################################
//
//    Caffa
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013-2022 Ceetron Solutions AS
//    Copyright (C) Changes 2022- Kontur AS
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
#include "cafField.h"
#include "cafObject.h"
#include "cafStringTools.h"

#include <string>

namespace caffa
{
//==================================================================================================
/// The Document class is the main class to do file based IO,
/// and is also supposed to act as the overall container of the objects read.
//==================================================================================================
class Document : public Object
{
    CAFFA_HEADER_INIT_WITH_DOC( "The Document class is a top level object acting as a "
                                "\"Project\" or container",
                                Document,
                                Object );

public:
    Document( const std::string& id = std::string( classKeywordStatic() ) );
    ~Document() noexcept override;

    std::string id() const;
    std::string fileName() const;

    void setId( const std::string& id );
    void setFileName( const std::string& fileName );

    bool readFromJsonFile();
    bool writeToJsonFile() const;

private:
    Field<std::string> m_id;
    Field<std::string> m_fileName;
};

} // End of namespace caffa
