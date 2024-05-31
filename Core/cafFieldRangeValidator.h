#pragma once

#include "cafFieldValidator.h"

#include "nlohmann/json.hpp"

namespace caffa
{
/**
 * @brief Simple range validator.
 *
 * @tparam DataType
 */
template <typename DataType>
class FieldRangeValidator : public FieldValidator<DataType>
{
public:
    using FailureSeverity = FieldValidatorInterface::FailureSeverity;

    FieldRangeValidator( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
        : FieldValidator<DataType>( failureSeverity )
        , m_minimum( minimum )
        , m_maximum( maximum )
    {
    }

    void readFromString( const std::string& string ) override
    {
        auto jsonFieldObject = nlohmann::json::parse( string );
        if ( jsonFieldObject.is_object() )
        {
            if ( jsonFieldObject.contains( "minimum" ) )
            {
                m_minimum = jsonFieldObject["minimum"];
            }
            if ( jsonFieldObject.contains( "maximum" ) )
            {
                m_maximum = jsonFieldObject["maximum"];
            }
        }
    }

    void writeToString( std::string& string ) const override
    {
        nlohmann::json jsonFieldObject = nlohmann::json::parse( string );

        jsonFieldObject["minimum"] = m_minimum;
        jsonFieldObject["maximum"] = m_maximum;

        string = jsonFieldObject.dump();
    }

    std::pair<bool, std::string> validate( const DataType& value ) const override
    {
        bool valid = m_minimum <= value && value <= m_maximum;
        if ( !valid )
        {
            std::stringstream ss;
            ss << "The value " << value << " is outside the limits [" << m_minimum << ", " << m_maximum << "]";
            return std::make_pair( false, ss.str() );
        }
        return std::make_pair( true, "" );
    }

    static std::unique_ptr<FieldRangeValidator<DataType>>
        create( DataType minimum, DataType maximum, FailureSeverity failureSeverity = FailureSeverity::VALIDATOR_ERROR )
    {
        return std::make_unique<FieldRangeValidator<DataType>>( minimum, maximum, failureSeverity );
    }

private:
    DataType m_minimum;
    DataType m_maximum;
};
} // namespace caffa