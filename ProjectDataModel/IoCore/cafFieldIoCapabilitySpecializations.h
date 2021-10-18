#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"

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

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldIoCapability>() )
    {
        new FieldIoCap<FieldType>( field, true );
    }
}

} // End of namespace caffa

#include "cafFieldIoCapabilitySpecializations.inl"
