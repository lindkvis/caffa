
#include "gtest/gtest.h"

#include "cafPdmDataValueField.h"
#include "cafPdmFieldIoCapabilitySpecializations.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectHandleIoMacros.h"
#include "cafPdmObjectXmlCapability.h"

#include <QXmlStreamWriter>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SimpleObjectWithNumbers : public caf::PdmObjectHandle, public caf::PdmObjectIoCapability
{
    CAF_PDM_IO_HEADER_INIT;

public:
    SimpleObjectWithNumbers()
        : PdmObjectHandle()
        , PdmObjectIoCapability( this, false )
    {
        CAF_PDM_IO_InitField( &m_valueA, "ValueA" );
        CAF_PDM_IO_InitField( &m_valueB, "ValueB" );

        CAF_PDM_IO_InitField( &m_floatValueA, "FloatValueA" );
        CAF_PDM_IO_InitField( &m_floatValueB, "FloatValueB" );
    }

    caf::PdmDataValueField<double> m_valueA;
    caf::PdmDataValueField<double> m_valueB;

    caf::PdmDataValueField<float> m_floatValueA;
    caf::PdmDataValueField<float> m_floatValueB;
};
CAF_PDM_IO_SOURCE_INIT( SimpleObjectWithNumbers, "SimpleObjectWithNumbers" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithDoubleValues )
{
    double valueA = 0.123456789;
    double valueB = 123456789 + valueA;

    QString objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_valueA = valueA;
        obj1.m_valueB = valueB;

        objectAsText = obj1.writeObjectToString();
    }

    {
        SimpleObjectWithNumbers obj1;

        obj1.readObjectFromString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

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

    QString objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_floatValueA = valueA;
        obj1.m_floatValueB = valueB;

        objectAsText = obj1.writeObjectToString();
    }

    {
        SimpleObjectWithNumbers obj1;

        obj1.readObjectFromString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

        double epsilon = 1e-7;

        double diffA = fabs( obj1.m_floatValueA - valueA );
        EXPECT_TRUE( diffA < epsilon );

        double diffB = fabs( obj1.m_floatValueB - valueB );
        EXPECT_TRUE( diffB < epsilon );
    }
}
