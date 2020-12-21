#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafValueFieldSpecializations.h"
#include "cafVariant.h"

#include <Wt/WDate.h>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
template <>
class ValueFieldSpecialization<Wt::WDate>
{
public:
    static Variant convert( const Wt::WDate& date );

    static void setFromVariant( const Variant& variantValue, Wt::WDate& value );

    static bool isEqual( const Variant& variantValue, const Variant& variantValue2 );
};

} // namespace caf
#ifdef _MSC_VER
#pragma warning( pop )
#endif
