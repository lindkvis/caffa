
#include "gtest.h"

#include "cafDataValueField.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafObjectHandle.h"
#include "cafObjectHandleIoMacros.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SimpleObjectWithNumbers : public caffa::ObjectHandle, public caffa::ObjectIoCapability
{
    CAF_IO_HEADER_INIT;

public:
    SimpleObjectWithNumbers()
        : ObjectHandle()
        , ObjectIoCapability( this, false )
    {
        CAF_IO_InitField( &m_valueA, "ValueA" );
        CAF_IO_InitField( &m_valueB, "ValueB" );

        CAF_IO_InitField( &m_floatValueA, "FloatValueA" );
        CAF_IO_InitField( &m_floatValueB, "FloatValueB" );
    }

    caffa::DataValueField<double> m_valueA;
    caffa::DataValueField<double> m_valueB;

    caffa::DataValueField<float> m_floatValueA;
    caffa::DataValueField<float> m_floatValueB;
};
CAF_IO_SOURCE_INIT( SimpleObjectWithNumbers, "SimpleObjectWithNumbers", "" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithDoubleValues )
{
    double valueA = 0.123456789;
    double valueB = 123456789 + valueA;

    std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string objectAsText;

        {
            SimpleObjectWithNumbers obj1;

            obj1.m_valueA = valueA;
            obj1.m_valueB = valueB;

            objectAsText = obj1.writeObjectToString( ioType );
        }

        {
            SimpleObjectWithNumbers obj1;

            obj1.readObjectFromString( objectAsText, caffa::DefaultObjectFactory::instance(), ioType );

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithFloatValues )
{
    float valueA = 0.123456789f;
    float valueB = 123456 + valueA;

    std::vector<caffa::ObjectIoCapability::IoType> ioTypes = { caffa::ObjectIoCapability::IoType::JSON };

    for ( auto ioType : ioTypes )
    {
        std::string objectAsText;

        {
            SimpleObjectWithNumbers obj1;

            obj1.m_floatValueA = valueA;
            obj1.m_floatValueB = valueB;

            objectAsText = obj1.writeObjectToString( ioType );
        }

        {
            SimpleObjectWithNumbers obj1;

            obj1.readObjectFromString( objectAsText, caffa::DefaultObjectFactory::instance(), ioType );

            double epsilon = 1e-7;

            double diffA = fabs( obj1.m_floatValueA - valueA );
            EXPECT_TRUE( diffA < epsilon );

            double diffB = fabs( obj1.m_floatValueB - valueB );
            EXPECT_TRUE( diffB < epsilon );
        }
    }
}
