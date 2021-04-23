#pragma once

#include "cafAppEnum.h"

#include <nlohmann/json.hpp>

#include <sstream>

namespace caffa
{
template <typename T>
void to_json( nlohmann::json& jsonValue, const AppEnum<T>& appEnum )
{
    std::stringstream stream;
    stream << appEnum;
    jsonValue = stream.str();
}

template <typename T>
void from_json( const nlohmann::json& jsonValue, AppEnum<T>& appEnum )
{
    std::stringstream stream( jsonValue.get<std::string>() );
    stream >> appEnum;
}
} // End of namespace caffa
