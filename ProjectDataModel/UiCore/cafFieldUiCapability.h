#pragma once

#include "cafFieldCapability.h"
#include "cafUiItem.h"
#include "cafVariant.h"

#include <deque>
#include <list>

namespace caffa
{
class FieldHandle;

class FieldUiCapability : public UiItem, public FieldCapability
{
public:
    FieldUiCapability( FieldHandle* owner, bool giveOwnership );
    ~FieldUiCapability() override;

    FieldHandle* fieldHandle();

    // Generalized access methods for User interface
    // The Variant encapsulates the real value, or an index into the valueOptions

    virtual Variant                    uiValue() const;
    virtual std::deque<OptionItemInfo> valueOptions( bool* useOptionsOnly ) const;

    void notifyFieldChanged( const FieldCapability* changedByCapability,
                             const Variant&         oldValue,
                             const Variant&         newValue ) override;

    virtual Variant toUiBasedVariant() const = 0;

private:
    friend class UiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void setValueFromUiEditor( const Variant& uiValue );
    // This is needed to handle custom types in Variants since operator == between Variant does not work when they use
    // custom types.
    virtual bool isVariantDataEqual( const Variant& oldUiBasedVariant, const Variant& newUiBasedVariant ) const;

private:
    FieldHandle* m_owner;
};

} // End of namespace caffa
