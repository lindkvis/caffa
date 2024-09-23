#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"

#include <nlohmann/json.hpp>
#include <sstream>

namespace caffa
{
class JsonSerializer;

template <typename FieldType>
class FieldIoCap final : public FieldIoCapability
{
public:
    FieldIoCap() = default;

    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    [[nodiscard]] nlohmann::json jsonType() const override;

private:
    FieldType* typedOwner() const { return dynamic_cast<FieldType*>( this->owner() ); }
};

template <typename DataType>
class FieldIoCap<ChildField<DataType*>> final : public FieldIoCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldIoCap() = default;

    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    [[nodiscard]] nlohmann::json jsonType() const override;

private:
    FieldType* typedOwner() const { return dynamic_cast<FieldType*>( this->owner() ); }
};

template <typename DataType>
class FieldIoCap<ChildArrayField<DataType*>> : public FieldIoCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldIoCap() = default;

    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    [[nodiscard]] nlohmann::json jsonType() const override;

private:
    FieldType* typedOwner() const { return dynamic_cast<FieldType*>( this->owner() ); }
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldIoCapability>() )
    {
        field->addCapability( std::make_unique<FieldIoCap<FieldType>>() );
    }
}

} // namespace caffa

#include "cafFieldIoCapabilitySpecializations.inl"
