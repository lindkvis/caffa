#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"
#include "cafInternalIoFieldReaderWriter.h"
#include "cafPtrArrayField.h"
#include "cafPtrField.h"

#include <nlohmann/json.hpp>

#include <string>
#include <typeinfo>

namespace caf
{
template <typename FieldType>
class FieldIoCap : public FieldIoCapability
{
public:
    // Type traits magic to check if a template argument is a vector
    template <typename T>
    struct is_vector : public std::false_type
    {
    };
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = typeid( typename FieldType::FieldDataType ).name();
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class PtrField;

template <typename DataType>
class FieldIoCap<PtrField<DataType*>> : public FieldIoCapability
{
    typedef PtrField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool        resolveReferences() override;
    std::string referenceString() const override;

private:
    FieldType* m_field;

    // Resolving
    std::string m_referenceString;
    bool        m_isResolved;
};

template <typename DataType>
class PtrArrayField;

template <typename DataType>
class FieldIoCap<PtrArrayField<DataType*>> : public FieldIoCapability
{
    typedef PtrArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool        resolveReferences() override;
    std::string referenceString() const override;
    bool        isVectorField() const override;

private:
    FieldType* m_field;

    // Resolving
    std::string m_referenceString;
    bool        m_isResolved;
};

template <typename DataType>
class ChildField;

template <typename DataType>
class FieldIoCap<ChildField<DataType*>> : public FieldIoCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool resolveReferences() override;

private:
    FieldType* m_field;
};

template <typename DataType>
class ChildArrayField;

template <typename DataType>
class FieldIoCap<ChildArrayField<DataType*>> : public FieldIoCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class Field;

template <typename DataType>
class FieldIoCap<Field<std::vector<DataType>>> : public FieldIoCapability
{
    typedef Field<std::vector<DataType>> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field = field;

        m_dataTypeName = typeid( DataType ).name();
    }

public:
    // Json Serializing
    void readFieldData( const nlohmann::json& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( nlohmann::json& jsonValue, bool writeServerAddress ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
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

} // End of namespace caf

#include "cafFieldIoCapabilitySpecializations.inl"
