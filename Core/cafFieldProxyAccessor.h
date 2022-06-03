#pragma once

#include "cafAssert.h"
#include "cafField.h"
#include "cafObservingPointer.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace caffa
{
template <typename DataType>
class SetValueInterface
{
public:
    virtual ~SetValueInterface() {}
    virtual void                                         setValue( const DataType& value ) = 0;
    virtual std::unique_ptr<SetValueInterface<DataType>> clone() const                     = 0;
};

template <typename DataType>
class SetterMethodCB : public SetValueInterface<DataType>
{
public:
    using SetterMethodType = std::function<void( const DataType& )>;

    SetterMethodCB( SetterMethodType setterMethod ) { m_setterMethod = setterMethod; }

    void setValue( const DataType& value )
    {
        CAFFA_ASSERT( m_setterMethod );
        m_setterMethod( value );
    }

    virtual std::unique_ptr<SetValueInterface<DataType>> clone() const
    {
        return std::make_unique<SetterMethodCB<DataType>>( m_setterMethod );
    }

private:
    SetterMethodType m_setterMethod;
};

template <typename DataType>
class GetValueInterface
{
public:
    virtual ~GetValueInterface() {}
    virtual DataType                                     getValue() const = 0;
    virtual std::unique_ptr<GetValueInterface<DataType>> clone() const    = 0;
};

template <typename DataType>
class GetterMethodCB : public GetValueInterface<DataType>
{
public:
    using GetterMethodType = std::function<DataType()>;

    GetterMethodCB( GetterMethodType setterMethod ) { m_getterMethod = setterMethod; }

    DataType getValue() const { return m_getterMethod(); }

    virtual std::unique_ptr<GetValueInterface<DataType>> clone() const
    {
        return std::make_unique<GetterMethodCB<DataType>>( m_getterMethod );
    }

private:
    GetterMethodType m_getterMethod;
};

template <typename DataType>
class FieldProxyAccessor : public DataFieldAccessor<DataType>
{
public:
    std::unique_ptr<DataFieldAccessor<DataType>> clone() const override
    {
        auto copy           = std::make_unique<FieldProxyAccessor>();
        copy->m_valueSetter = std::move( m_valueSetter->clone() );
        copy->m_valueGetter = std::move( m_valueGetter->clone() );
        return copy;
    }

    DataType value() override
    {
        CAFFA_ASSERT( m_valueGetter );
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
    void registerSetMethod( typename SetterMethodCB<DataType>::SetterMethodType setterMethod )
    {
        m_valueSetter = std::make_unique<SetterMethodCB<DataType>>( setterMethod );
    }

    void registerGetMethod( typename GetterMethodCB<DataType>::GetterMethodType getterMethod )
    {
        m_valueGetter = std::make_unique<GetterMethodCB<DataType>>( getterMethod );
    }

    bool hasSetter() const { return m_valueSetter != nullptr; }
    bool hasGetter() const { return m_valueGetter != nullptr; }

private:
    std::unique_ptr<SetValueInterface<DataType>> m_valueSetter;
    std::unique_ptr<GetValueInterface<DataType>> m_valueGetter;
};

} // End of namespace caffa
