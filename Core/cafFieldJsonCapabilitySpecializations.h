#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldJsonCapability.h"

#include <nlohmann/json.hpp>

#include <concepts>
#include <string>
#include <typeinfo>

namespace caffa
{
class JsonSerializer;

template <typename FieldType>
class FieldJsonCap : public FieldJsonCapability
{
public:
    FieldJsonCap()
        : FieldJsonCapability()
    {
    }

public:
    // Json Serializing
    void               readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void               writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;
    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldJsonCap<ChildField<DataType*>> : public FieldJsonCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldJsonCap()
        : FieldJsonCapability()
    {
    }

public:
    // Json Serializing
    void               readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void               writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;
    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldJsonCap<ChildArrayField<DataType*>> : public FieldJsonCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldJsonCap()
        : FieldJsonCapability()
    {
    }

public:
    // Json Serializing
    void               readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void               writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;
    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldJsonCapability>() )
    {
        field->addCapability( std::make_unique<FieldJsonCap<FieldType>>() );
    }
}

} // End of namespace caffa

#include "cafFieldJsonCapabilitySpecializations.inl"
