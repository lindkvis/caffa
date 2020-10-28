#pragma once

#include "cafPdmFieldUiCapability.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"

namespace caf
{
template <typename FieldType>
class PdmFieldUiCap : public PdmFieldUiCapability
{
public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmFieldUiCapability( field, giveOwnership )
    {
        m_field = field;
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override;
    void                     setValueFromUiEditor( const QVariant& uiValue ) override;
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override;

    QVariant toUiBasedQVariant() const override;

private:
    bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const override;

    mutable QList<PdmOptionItemInfo> m_optionEntryCache;

private:
    FieldType* m_field;
};

//
// Specialization for ChildFields to do nothing towards GUI
//
template <typename DataType>
class PdmFieldUiCap<PdmChildField<DataType*>> : public PdmFieldUiCapability
{
    typedef PdmChildField<DataType*> FieldType;

public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmFieldUiCapability( field, giveOwnership )
    {
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue ) override {}
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
};

//
// Specialization for ChildArrayFields to do nothing towards GUI
//
template <typename DataType>
class PdmFieldUiCap<PdmChildArrayField<DataType*>> : public PdmFieldUiCapability
{
    typedef PdmChildArrayField<DataType*> FieldType;

public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmFieldUiCapability( field, giveOwnership )
    {
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue ) override {}
    QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly ) const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
};

template <typename FieldType>
void AddUiCapabilityToField( FieldType* field )
{
    if ( field->template capability<PdmFieldUiCap<FieldType>>() == NULL )
    {
        new PdmFieldUiCap<FieldType>( field, true );
    }
}

} // End of namespace caf

#include "cafInternalPdmUiFieldCapability.inl"
