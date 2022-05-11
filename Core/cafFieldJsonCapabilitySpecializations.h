#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldJsonCapability.h"

#include <nlohmann/json.hpp>

#include <string>
#include <typeinfo>

namespace caffa
{
class Serializer;

template <typename FieldType>
class FieldJsonCap : public FieldJsonCapability
{
public:
    FieldJsonCap( FieldType* field, bool giveOwnership )
        : FieldJsonCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const Serializer& serializer ) const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldJsonCap<ChildField<DataType*>> : public FieldJsonCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldJsonCap( FieldType* field, bool giveOwnership )
        : FieldJsonCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const Serializer& serializer ) const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldJsonCap<ChildArrayField<DataType*>> : public FieldJsonCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldJsonCap( FieldType* field, bool giveOwnership )
        : FieldJsonCapability( field, giveOwnership )
    {
        m_field = field;
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const Serializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const Serializer& serializer ) const override;

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldJsonCapability>() )
    {
        new FieldJsonCap<FieldType>( field, true );
    }
}

} // End of namespace caffa

#include "cafFieldJsonCapabilitySpecializations.inl"
