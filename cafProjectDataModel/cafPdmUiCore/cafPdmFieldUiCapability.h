#pragma once

#include "cafPdmUiCore_export.h"

#include "cafPdmFieldCapability.h"
#include "cafPdmFieldUiCapabilityInterface.h"
#include "cafPdmUiItem.h"

namespace caf
{
class PdmFieldHandle;

class cafPdmUiCore_EXPORT PdmFieldUiCapability : public PdmUiItem, public PdmFieldCapability, public PdmFieldUiCapabilityInterface
{
public:
    PdmFieldUiCapability( PdmFieldHandle* owner, bool giveOwnership );
    ~PdmFieldUiCapability() override;

    PdmFieldHandle* fieldHandle();

    // Generalized access methods for User interface
    // The QVariant encapsulates the real value, or an index into the valueOptions

    virtual QVariant                 uiValue() const;
    virtual QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const;

    void notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) override;

    bool isAutoAddingOptionFromValue() const;
    void setAutoAddingOptionFromValue( bool isAddingValue );

private:
    friend class PdmUiCommandSystemProxy;
    friend class CmdFieldChangeExec;
    virtual void setValueFromUiEditor( const QVariant& uiValue );
    // This is needed to handle custom types in QVariants since operator == between QVariant does not work when they use
    // custom types.
    virtual bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const;

private:
    PdmFieldHandle* m_owner;
    bool            m_isAutoAddingOptionFromValue;
};

} // End of namespace caf
