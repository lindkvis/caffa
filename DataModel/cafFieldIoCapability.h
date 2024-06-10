#pragma once

#include "cafFieldCapability.h"

#include <string>

namespace caffa
{
class Serializer;
//==================================================================================================
//
//
//
//==================================================================================================
class FieldIoCapability : public FieldCapability
{
public:
    virtual void readFromString( const std::string& value, const Serializer& serializer ) = 0;
    virtual void writeToString( std::string& value, const Serializer& serializer ) const  = 0;
};
} // End of namespace caffa
