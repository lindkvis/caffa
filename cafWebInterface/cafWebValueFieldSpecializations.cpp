#include "cafWebValueFieldSpecializations.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant ValueFieldSpecialization<Wt::WDate>::convert( const Wt::WDate& date )
{
    int julianDay = date.toJulianDay();
    return Variant( julianDay );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ValueFieldSpecialization<Wt::WDate>::setFromVariant( const Variant& variantValue, Wt::WDate& value )
{
    int julianDay = variantValue.value<int>();
    value == Wt::WDate::fromJulianDay( julianDay );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ValueFieldSpecialization<Wt::WDate>::isEqual( const Variant& variantValue, const Variant& variantValue2 )
{
    return variantValue == variantValue2;
}
