#pragma once

#include "cafInternalPdmStreamOperators.h"
#include "cafPdmReferenceHelper.h"

#include <nlohmann/json.hpp>

#include <sstream>

namespace caf
{
class ObjectFactory;
template <typename T>
class PdmPointer;

//--------------------------------------------------------------------------------------------------
/// Generic write method for fields. Will work as long as DataType supports the stream operator
/// towards a iostream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
/// Implemented in a proxy class to allow  partial specialization
//--------------------------------------------------------------------------------------------------
template <typename DataType>
struct FieldWriter
{
    static void writeFieldData( const DataType& fieldValue, nlohmann::json& jsonValue )
    {
        std::stringstream stream;
        // Use precision of 15 to cover most value ranges for double values
        stream.precision( 15 );
        stream << fieldValue;
        jsonValue = stream.str();
    }
};

template <typename DataType>
struct FieldReader
{
    static void readFieldData( DataType& fieldValue, const nlohmann::json& jsonValue, ObjectFactory* objectFactory );
};

//--------------------------------------------------------------------------------------------------
/// Generic read method for fields. Will work as long as DataType supports the stream operator
/// towards a iostream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
//--------------------------------------------------------------------------------------------------

template <typename DataType>
void FieldReader<DataType>::readFieldData( DataType& fieldValue, const nlohmann::json& jsonValue, ObjectFactory* objectFactory )
{
    std::stringstream stream( jsonValue.get<std::string>() );
    stream >> fieldValue;
}

//--------------------------------------------------------------------------------------------------
/// Specialized read function for std::strings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void FieldReader<std::string>::readFieldData( std::string&          field,
                                              const nlohmann::json& jsonValue,
                                              ObjectFactory*        objectFactory );

} // End of namespace caf
