#pragma once

#include <QVariant>

namespace caf
{
class PdmFieldUiCapabilityInterface
{
public:
    PdmFieldUiCapabilityInterface() {}
    virtual ~PdmFieldUiCapabilityInterface() {}

    virtual QVariant toUiBasedQVariant() const { return QVariant(); }
    virtual void     notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ){};
};

} // End of namespace caf
