#pragma once

#include "cafAssert.h"
#include "cafField.h"

#include <functional>

namespace caffa
{
template <typename DataType>
class SetValueInterface
{
public:
    virtual ~SetValueInterface()                                                 = default;
    virtual void                               setValue( const DataType& value ) = 0;
    virtual std::unique_ptr<SetValueInterface> clone() const                     = 0;
};

template <typename DataType>
class SetterMethodCB final : public SetValueInterface<DataType>
{
public:
    using SetterMethodType = std::function<void( const DataType& )>;

    explicit SetterMethodCB( SetterMethodType setterMethod ) { m_setterMethod = setterMethod; }

    void setValue( const DataType& value ) override
    {
        if ( !m_setterMethod ) throw std::runtime_error( "No setter in Proxy Accessor" );
        m_setterMethod( value );
    }

    std::unique_ptr<SetValueInterface<DataType>> clone() const override
    {
        return std::make_unique<SetterMethodCB>( m_setterMethod );
    }

private:
    SetterMethodType m_setterMethod;
};

template <typename DataType>
class GetValueInterface
{
public:
    virtual ~GetValueInterface()                                = default;
    virtual DataType                           getValue() const = 0;
    virtual std::unique_ptr<GetValueInterface> clone() const    = 0;
};

template <typename DataType>
class GetterMethodCB final : public GetValueInterface<DataType>
{
public:
    using GetterMethodType = std::function<DataType()>;

    explicit GetterMethodCB( GetterMethodType setterMethod ) { m_getterMethod = setterMethod; }

    DataType getValue() const override { return m_getterMethod(); }

    std::unique_ptr<GetValueInterface<DataType>> clone() const override
    {
        return std::make_unique<GetterMethodCB>( m_getterMethod );
    }

private:
    GetterMethodType m_getterMethod;
};

template <typename DataType>
class FieldProxyAccessor : public DataFieldAccessor<DataType>
{
public:
    using GetterMethod = typename GetterMethodCB<DataType>::GetterMethodType;
    using SetterMethod = typename SetterMethodCB<DataType>::SetterMethodType;

    static std::unique_ptr<FieldProxyAccessor<DataType>> create( GetterMethod getterMethod, SetterMethod setterMethod )
    {
        auto accessor = std::make_unique<FieldProxyAccessor<DataType>>();
        accessor->registerGetMethod( getterMethod );
        accessor->registerSetMethod( setterMethod );
        return accessor;
    }

    std::unique_ptr<DataFieldAccessor<DataType>> clone() const override
    {
        auto copy           = std::make_unique<FieldProxyAccessor>();
        copy->m_valueSetter = std::move( m_valueSetter->clone() );
        copy->m_valueGetter = std::move( m_valueGetter->clone() );
        return copy;
    }

    DataType value() override
    {
        if ( !m_valueGetter ) throw std::runtime_error( "No getter for field" );
        return m_valueGetter->getValue();
    }

    void setValue( const DataType& value ) override
    {
        if ( !m_valueSetter ) throw std::runtime_error( "No setter for field" );
        m_valueSetter->setValue( value );
    }

    // Proxy Field stuff to handle the method pointers
    // The public registering methods must be written below the private classes
    // For some reason. Forward declaration did some weirdness.
private:
public:
    void registerSetMethod( SetterMethod setterMethod )
    {
        m_valueSetter = std::make_unique<SetterMethodCB<DataType>>( setterMethod );
    }

    void registerGetMethod( GetterMethod getterMethod )
    {
        m_valueGetter = std::make_unique<GetterMethodCB<DataType>>( getterMethod );
    }

    [[nodiscard]] bool hasSetter() const override { return m_valueSetter != nullptr; }
    [[nodiscard]] bool hasGetter() const override { return m_valueGetter != nullptr; }

private:
    std::unique_ptr<SetValueInterface<DataType>> m_valueSetter;
    std::unique_ptr<GetValueInterface<DataType>> m_valueGetter;
};

} // End of namespace caffa
