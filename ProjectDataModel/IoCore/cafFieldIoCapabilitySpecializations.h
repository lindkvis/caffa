#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"
#include "cafFifoField.h"
#include "cafPtrArrayField.h"
#include "cafPtrField.h"

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
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool resolveReferences() override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<PtrField<DataType*>> : public FieldIoCapability
{
    typedef PtrField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_isResolved      = false;
        m_referenceString = "";
    }

public:
    // Json Serializing
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool        resolveReferences() override;
    std::string referenceString() const override;

private:
    FieldType* m_field;

    // Resolving
    std::string m_referenceString;
    bool        m_isResolved;
};

template <typename DataType>
class FieldIoCap<PtrArrayField<DataType*>> : public FieldIoCapability
{
    typedef PtrArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_isResolved      = false;
        m_referenceString = "";
    }

public:
    // Json Serializing
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool        resolveReferences() override;
    std::string referenceString() const override;

private:
    FieldType* m_field;

    // Resolving
    std::string m_referenceString;
    bool        m_isResolved;
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
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool resolveReferences() override;

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
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool resolveReferences() override;

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
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool resolveReferences() override;

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
    void writeToField( const nlohmann::json& jsonValue, ObjectFactory* objectFactory, bool copyDataValues ) override;
    void readFromField( nlohmann::json& jsonValue, bool copyServerAddress, bool copyDataValues ) const override;

    bool resolveReferences() override;

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
