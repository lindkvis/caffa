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

#include "cafObjectJsonConversion.h"
#include "cafJsonSerializer.h"

namespace caffa
{

void tag_invoke( boost::json::value_from_tag, boost::json::value& v, const std::shared_ptr<ObjectHandle>& objectHandle )
{
    json::object object;
    JsonSerializer().writeObjectToJson( objectHandle.get(), object );
    v = object;
}

void tag_invoke( boost::json::value_from_tag, boost::json::value& v, const ObjectHandle& objectHandle )
{
    json::object object;
    JsonSerializer().writeObjectToJson( &objectHandle, object );
    v = object;
}

std::shared_ptr<ObjectHandle> tag_invoke( boost::json::value_to_tag<std::shared_ptr<ObjectHandle>>, const json::value& v )
{
    if ( const auto* jsonObject = v.if_object(); jsonObject )
    {
        return JsonSerializer().createObjectFromJson( *jsonObject );
    }
    throw std::runtime_error( "Failed to create object handle from JSON" );
}

} // namespace caffa