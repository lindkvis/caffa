#pragma once

namespace caf
{
template <class DataType>
class DataFieldAccessorInterface
{
public:
    virtual ~DataFieldAccessorInterface() = default;

    virtual std::unique_ptr<DataFieldAccessorInterface> clone() const = 0;

    virtual DataType&       value()                           = 0;
    virtual const DataType& value() const                     = 0;
    virtual void            setValue( const DataType& value ) = 0;
    virtual void            reset()                           = 0;

    virtual std::optional<DataType> defaultValue() const                     = 0;
    virtual void                    setDefaultValue( const DataType& value ) = 0;
};

template <class DataType>
class DataFieldDirectStorageAccessor : public DataFieldAccessorInterface<DataType>
{
public:
    DataFieldDirectStorageAccessor() = default;
    DataFieldDirectStorageAccessor( const DataType& defaultValue )
        : m_defaultValue( defaultValue )
    {
        reset();
    }

    std::unique_ptr<DataFieldAccessorInterface<DataType>> clone() const override
    {
        std::unique_ptr<DataFieldAccessorInterface<DataType>> copy;
        if ( m_defaultValue.has_value() )
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>( *m_defaultValue );
        else
            copy = std::make_unique<DataFieldDirectStorageAccessor<DataType>>();

        copy->setValue( m_value );
        return copy;
    }

    DataType&       value() override { return m_value; };
    const DataType& value() const override { return m_value; };

    void setValue( const DataType& value ) override { m_value = value; }
    void reset() override
    {
        if ( m_defaultValue.has_value() ) m_value = *m_defaultValue;
    }
    std::optional<DataType> defaultValue() const override { return m_defaultValue; }
    void                    setDefaultValue( const DataType& value ) override { m_defaultValue = value; }

private:
    DataType                m_value;
    std::optional<DataType> m_defaultValue;
};

} // namespace caf
