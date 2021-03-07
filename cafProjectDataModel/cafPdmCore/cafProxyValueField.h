#pragma once

#include "cafAssert.h"
#include "cafDataValueField.h"
#include "cafPdmPointer.h"
#include "cafValueField.h"
#include "cafValueFieldSpecializations.h"

#include <type_traits>
#include <vector>

namespace caf
{
//==================================================================================================
/// Abstract non-templated base class for ProxyValueField
/// Exists only to be able to determine that a field is a proxy field
//==================================================================================================

class ProxyFieldHandle
{
public:
    virtual bool isStreamingField() const = 0;
    virtual bool hasGetter() const        = 0;
    virtual bool hasSetter() const        = 0;
};

template <typename DataType>
class SetValueInterface
{
public:
    virtual ~SetValueInterface() {}
    virtual void                                         setValue( const DataType& value ) = 0;
    virtual std::unique_ptr<SetValueInterface<DataType>> clone() const                     = 0;
};

template <typename DataType, typename ObjectType>
class SetterMethodCB : public SetValueInterface<DataType>
{
public:
    typedef void ( ObjectType::*SetterMethodType )( const DataType& value );

    SetterMethodCB( ObjectType* obj, SetterMethodType setterMethod )
    {
        m_setterMethod = setterMethod;
        m_obj          = obj;
    }

    void setValue( const DataType& value ) { ( m_obj->*m_setterMethod )( value ); }

    virtual std::unique_ptr<SetValueInterface<DataType>> clone() const
    {
        return std::make_unique<SetterMethodCB<DataType, ObjectType>>( m_obj, m_setterMethod );
    }

private:
    SetterMethodType       m_setterMethod;
    PdmPointer<ObjectType> m_obj;
};

template <typename DataType>
class GetValueInterface
{
public:
    virtual ~GetValueInterface() {}
    virtual DataType                                     getValue() const = 0;
    virtual std::unique_ptr<GetValueInterface<DataType>> clone() const    = 0;
};

template <typename DataType, typename ObjectType>
class GetterMethodCB : public GetValueInterface<DataType>
{
public:
    typedef DataType ( ObjectType::*GetterMethodType )() const;

    GetterMethodCB( ObjectType* obj, GetterMethodType setterMethod )
    {
        m_getterMethod = setterMethod;
        m_obj          = obj;
    }

    DataType getValue() const { return ( m_obj->*m_getterMethod )(); }

    virtual std::unique_ptr<GetValueInterface<DataType>> clone() const
    {
        return std::make_unique<GetterMethodCB<DataType, ObjectType>>( m_obj, m_getterMethod );
    }

private:
    GetterMethodType       m_getterMethod;
    PdmPointer<ObjectType> m_obj;
};

template <typename DataType>
class ProxyFieldDataAccessor : public DataFieldAccessor<DataType>
{
public:
    std::unique_ptr<DataFieldAccessor<DataType>> clone() const override
    {
        auto copy           = std::make_unique<ProxyFieldDataAccessor>();
        copy->m_valueSetter = std::move( m_valueSetter->clone() );
        copy->m_valueGetter = std::move( m_valueGetter->clone() );
        return copy;
    }

    DataType value() override
    {
        CAF_ASSERT( m_valueGetter );
        return m_valueGetter->getValue();
    }

    void setValue( const DataType& value ) override
    {
        if ( m_valueSetter ) m_valueSetter->setValue( value );
    }

    // Proxy Field stuff to handle the method pointers
    // The public registering methods must be written below the private classes
    // For some reason. Forward declaration did some weirdness.
private:
public:
    template <typename OwnerObjectType>
    void registerSetMethod( OwnerObjectType*                                                     obj,
                            typename SetterMethodCB<DataType, OwnerObjectType>::SetterMethodType setterMethod )
    {
        m_valueSetter = std::make_unique<SetterMethodCB<DataType, OwnerObjectType>>( obj, setterMethod );
    }

    template <typename OwnerObjectType>
    void registerGetMethod( OwnerObjectType*                                                     obj,
                            typename GetterMethodCB<DataType, OwnerObjectType>::GetterMethodType getterMethod )
    {
        m_valueGetter = std::make_unique<GetterMethodCB<DataType, OwnerObjectType>>( obj, getterMethod );
    }

    bool hasSetter() const { return m_valueSetter != nullptr; }
    bool hasGetter() const { return m_valueGetter != nullptr; }

private:
    std::unique_ptr<SetValueInterface<DataType>> m_valueSetter;
    std::unique_ptr<GetValueInterface<DataType>> m_valueGetter;
};

//==================================================================================================
/// Field class encapsulating data access through object setter/getter with input and output of this
/// data to/from a QXmlStream
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================
template <typename DataType>
class ProxyValueField : public DataValueField<DataType>, public ProxyFieldHandle
{
public:
    using ProxyAccessor = ProxyFieldDataAccessor<DataType>;

    ProxyValueField()
        : DataValueField<DataType>( std::make_unique<ProxyAccessor>() )
    {
    }

    bool isStreamingField() const override { return caf::is_vector<DataType>(); }
    bool hasGetter() const override { return proxyAccessor()->hasGetter(); }
    bool hasSetter() const override { return proxyAccessor()->hasSetter(); }

    bool isReadOnly() const override
    {
        if ( !hasSetter() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // Access operators

    DataType operator()() const { return this->value(); }
    DataType v() const { return this->value(); }
    bool     operator==( const DataType& otherValue ) const { return this->value() == otherValue; }

    template <typename OwnerObjectType>
    void registerSetMethod( OwnerObjectType*                                                     obj,
                            typename SetterMethodCB<DataType, OwnerObjectType>::SetterMethodType setterMethod )
    {
        proxyAccessor()->registerSetMethod( obj, setterMethod );
    }

    template <typename OwnerObjectType>
    void registerGetMethod( OwnerObjectType*                                                     obj,
                            typename GetterMethodCB<DataType, OwnerObjectType>::GetterMethodType getterMethod )
    {
        proxyAccessor()->registerGetMethod( obj, getterMethod );
    }

private:
    ProxyAccessor*       proxyAccessor() { return static_cast<ProxyAccessor*>( this->m_fieldDataAccessor.get() ); }
    const ProxyAccessor* proxyAccessor() const
    {
        return static_cast<const ProxyAccessor*>( this->m_fieldDataAccessor.get() );
    }

    PDM_DISABLE_COPY_AND_ASSIGN( ProxyValueField );
};

} // End of namespace caf
