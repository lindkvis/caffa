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
class Serializer;

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
    void               readFromString( const std::string& string, const Serializer& serializer ) override;
    void               writeToString( std::string& string, const Serializer& serializer ) const override;
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
    void               readFromString( const std::string& string, const Serializer& serializer ) override;
    void               writeToString( std::string& string, const Serializer& serializer ) const override;
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
    void               readFromString( const std::string& string, const Serializer& serializer ) override;
    void               writeToString( std::string& string, const Serializer& serializer ) const override;
    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

} // End of namespace caffa

#include "cafFieldJsonCapabilitySpecializations.inl"
