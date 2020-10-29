#pragma once

#include <QVariant>

namespace caf
{
class FieldUiCapabilityInterface
{
public:
    FieldUiCapabilityInterface() {}
    virtual ~FieldUiCapabilityInterface() {}

    virtual QVariant toUiBasedQVariant() const { return QVariant(); }
    virtual void     notifyFieldChanged( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ){};
};

} // End of namespace caf
