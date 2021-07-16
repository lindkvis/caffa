#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"
#include "cafFifoField.h"

#include <nlohmann/json.hpp>

#include <string>
#include <typeinfo>

namespace caffa
{
template <typename FieldType>
class FieldIoCap : public FieldIoCapability
{
public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<ChildField<DataType*>> : public FieldIoCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<ChildArrayField<DataType*>> : public FieldIoCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<FifoBlockingField<DataType>> : public FieldIoCapability
{
    using FieldType = FifoBlockingField<DataType>;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
        , m_readLimit( 10u )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const override;

    void   setMaximumPackagesForIoOutput( size_t maximumPackageCount ) { m_readLimit = maximumPackageCount; }
    size_t maximumPackagesForIoOutput() const { return m_readLimit; }

private:
    FieldType* m_field;
    size_t     m_readLimit;
};

template <typename DataType>
class FieldIoCap<FifoBoundedField<DataType>> : public FieldIoCapability
{
    using FieldType = FifoBoundedField<DataType>;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
        , m_readLimit( 10u )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void writeToJson( nlohmann::json& jsonValue, bool copyDataValues ) const override;

    void   setMaximumPackagesForIoOutput( size_t maximumPackageCount ) { m_readLimit = maximumPackageCount; }
    size_t maximumPackagesForIoOutput() const { return m_readLimit; }

private:
    FieldType* m_field;
    size_t     m_readLimit;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldIoCapability>() )
    {
        new FieldIoCap<FieldType>( field, true );
    }
}

template <typename FieldType>
void RegisterClassWithField( const std::string& classKeyword, FieldType* field )
{
    field->setOwnerClass( classKeyword );
}

} // End of namespace caffa

#include "cafFieldIoCapabilitySpecializations.inl"
