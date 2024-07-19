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
class FieldIoCap : public FieldIoCapability
{
public:
    FieldIoCap()
        : m_field( nullptr )
    {
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    void readFromString( const std::string& string ) override;
    void writeToString( std::string& string ) const override;

    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<ChildField<DataType*>> : public FieldIoCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldIoCap()
        : m_field( nullptr )
    {
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    void readFromString( const std::string& string ) override;
    void writeToString( std::string& string ) const override;

    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class FieldIoCap<ChildArrayField<DataType*>> : public FieldIoCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldIoCap()
        : m_field( nullptr )
    {
    }

public:
    // Json Serializing
    void readFromJson( const nlohmann::json& jsonElement, const JsonSerializer& serializer ) override;
    void writeToJson( nlohmann::json& jsonElement, const JsonSerializer& serializer ) const override;

    void readFromString( const std::string& string ) override;
    void writeToString( std::string& string ) const override;

    const FieldHandle* owner() const override;
    void               setOwner( FieldHandle* owner ) override;
    nlohmann::json     jsonType() const override;

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldIoCapability>() )
    {
        field->addCapability( std::make_unique<FieldIoCap<FieldType>>() );
    }
}

} // End of namespace caffa

namespace std
{
template <class T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& v )
{
    for ( typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii )
    {
        os << " " << *ii;
    }
    return os;
}

template <typename T>
std::istream& operator>>( std::istream& is, std::vector<T>& v )
{
    auto begin = std::istream_iterator<std::string>( is );
    auto end   = std::istream_iterator<std::string>();
    for ( auto it = begin; it != end; ++it )
    {
        std::stringstream ss( *it );
        T                 value{};
        ss >> value;
        v.push_back( value );
    }
    return is;
}

} // namespace std

#include "cafFieldIoCapabilitySpecializations.inl"
