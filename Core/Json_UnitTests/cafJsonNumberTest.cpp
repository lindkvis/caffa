
#include "gtest.h"

#include "cafField.h"
#include "cafFieldJsonCapabilitySpecializations.h"
#include "cafJsonSerializer.h"
#include "cafObject.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SimpleObjectWithNumbers : public caffa::Object
{
    CAFFA_HEADER_INIT( SimpleObjectWithNumbers, Object )

public:
    SimpleObjectWithNumbers()
    {
        initField( m_valueA, "ValueA" ).withCapability<caffa::FieldJsonCap<caffa::Field<double>>>();
        initField( m_valueB, "ValueB" ).withCapability<caffa::FieldJsonCap<caffa::Field<double>>>();
        ;

        initField( m_floatValueA, "FloatValueA" ).withCapability<caffa::FieldJsonCap<caffa::Field<float>>>();
        ;
        initField( m_floatValueB, "FloatValueB" ).withCapability<caffa::FieldJsonCap<caffa::Field<float>>>();
    }

    caffa::Field<double> m_valueA;
    caffa::Field<double> m_valueB;

    caffa::Field<float> m_floatValueA;
    caffa::Field<float> m_floatValueB;
};
CAFFA_SOURCE_INIT( SimpleObjectWithNumbers )

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithDoubleValues )
{
    double valueA = 0.123456789;
    double valueB = 123456789 + valueA;

    std::string objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_valueA = valueA;
        obj1.m_valueB = valueB;

        objectAsText = caffa::JsonSerializer().writeObjectToString( &obj1 );
    }

    {
        SimpleObjectWithNumbers obj1;

        caffa::JsonSerializer().readObjectFromString( &obj1, objectAsText );

        {
            double epsilon = 1e-7;

            double diffA = fabs( obj1.m_valueA - valueA );
            EXPECT_TRUE( diffA < epsilon );
        }

        {
            double epsilon = 3e-7;

            double diffB = fabs( obj1.m_valueB - valueB );
            EXPECT_TRUE( diffB < epsilon );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithFloatValues )
{
    float valueA = 0.123456789f;
    float valueB = 123456 + valueA;

    std::string objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_floatValueA = valueA;
        obj1.m_floatValueB = valueB;

        objectAsText = caffa::JsonSerializer().writeObjectToString( &obj1 );
    }

    {
        SimpleObjectWithNumbers obj1;

        caffa::JsonSerializer().readObjectFromString( &obj1, objectAsText );

        double epsilon = 1e-7;

        double diffA = fabs( obj1.m_floatValueA - valueA );
        EXPECT_TRUE( diffA < epsilon );

        double diffB = fabs( obj1.m_floatValueB - valueB );
        EXPECT_TRUE( diffB < epsilon );
    }
}
