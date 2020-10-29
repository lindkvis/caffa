#pragma once

#include "cafPdmUiCore_export.h"

#include "cafFieldCapability.h"
#include "cafFieldUiCapabilityInterface.h"
#include "cafUiItem.h"

namespace caf
{
class FieldHandle;

class cafPdmUiCore_EXPORT FieldUiCapability : public UiItem, public FieldCapability, public FieldUiCapabilityInterface
{
public:
    FieldUiCapability( FieldHandle* owner, bool giveOwnership );
    ~FieldUiCapability() override;

    FieldHandle* fieldHandle();

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant                 uiValue() const;
    virtual QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const;

    void notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) override;

    bool isAutoAddingOptionFromValue() const;
    void setAutoAddingOptionFromValue( bool isAddingValue );

private:
    friend class UiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void setValueFromUiEditor( const QVariant& uiValue );
    // This is needed to handle custom types in QVariants since operator == between QVariant does not work when they use
    // custom types.
    virtual bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const;

private:
    FieldHandle* m_owner;
    bool            m_isAutoAddingOptionFromValue;
};

} // End of namespace caf
