#include "cafInternalIoFieldReaderWriter.h"

#include <nlohmann/json.hpp>
#include <tinyxml2.h>

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Specialized read function for std::strings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void FieldReader<std::string>::readFieldData( std::string& field, const nlohmann::json& jsonValue, ObjectFactory* )
{
    field = jsonValue.get<std::string>();
}
} // End of namespace caf
