// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//    Copyright (C) 2013- Ceetron Solutions AS
//    Copyright (C) 2022- Kontur AS
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################

#pragma once

#include "cafAssert.h"
#include "cafLogger.h"
#include "cafPortableDataType.h"
#include "cafStringTools.h"

#include <nlohmann/json.hpp>

#include <concepts>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace caffa
{
template <typename T>
concept enum_type = std::is_enum<T>::value;

/**
 * An enum class wrapper to enable introspection and automatic I/O on enums.
 *
 * Usage, in header:
 * enum class SomeEnum
 *    {
 *       A = 2,
 *       B = 7
 *    };
 * caffa::AppEnum<SomeEnum, "SomeEnum"> myEnum;
 *
 * In cpp file:
 * namespace caffa {
 *    template<>
 *    void caffa::AppEnum<SomeEnum, "SomeEnum">::setUp()
 *    {
 *        addItem(SomeEnum::A,           "A");
 *        addItem(SomeEnum::B,           "B");
 *        setDefault(SomeEnum::B);
 *    }
 * }
 */
template <typename Enum>
    requires enum_type<Enum>
class AppEnum
{
public:
    using DataType = Enum;

    AppEnum()
    {
        setUp();
        m_value = m_defaultValue;
    }
    AppEnum( Enum value )
        : m_value( value )
    {
        setUp();
    }
    AppEnum( const std::string& value )
    {
        setUp();
        m_value = m_defaultValue;
        setFromLabel( value );
    }

    Enum value() const
    {
        if ( m_value )
        {
            return *m_value;
        }
        throw std::runtime_error( "The AppEnum has no value!" );
    }

    auto operator<=>( const AppEnum& rhs ) const = default;

    AppEnum& operator=( Enum value )
    {
        m_value = value;
        return *this;
    }

    void setFromLabel( const std::string& label )
    {
        auto value = enumVal( label );
        if ( value )
        {
            m_value = value;
        }
        else
        {
            throw std::runtime_error( label + " is not a valid option" );
        }
    }

    void setFromIndex( size_t index )
    {
        auto value = enumVal( index );
        if ( value )
        {
            m_value = value;
        }
        else
        {
            throw std::runtime_error( std::to_string( index ) + " is not a valid option index" );
        }
    }

    std::optional<Enum> enumVal( const std::string& label ) const
    {
        for ( auto [entryValue, entryLabel] : m_mapping )
        {
            if ( entryLabel == label )
            {
                return entryValue;
            }
        }

        CAFFA_ERROR( "No label " << label << " in AppEnum" );
        for ( const auto& [entry, entryLabel] : m_mapping )
        {
            CAFFA_ERROR( "Found label " << entryLabel << "(" << static_cast<int>( entry ) << ")" );
        }
        return std::nullopt;
    }

    std::optional<Enum> enumVal( size_t index ) const
    {
        if ( index < m_mapping.size() )
        {
            return m_mapping[index].first;
        }
        return std::nullopt;
    }

    size_t size() const { return m_mapping.size(); }

    std::vector<std::string> labels() const
    {
        std::vector<std::string> labelList;
        for ( const auto& [ignore, label] : m_mapping )
        {
            labelList.push_back( label );
        }
        return labelList;
    }

    std::string label() const { return this->label( value() ); }

    std::string label( Enum enumValue ) const
    {
        std::string label;
        for ( const auto& [entry, entryLabel] : m_mapping )
        {
            if ( enumValue == entry )
            {
                label = entryLabel;
                break;
            }
        }

        if ( label.empty() )
        {
            CAFFA_ERROR( "No value " << static_cast<int>( enumValue ) << " in AppEnum" );
            for ( const auto& [entry, entryLabel] : m_mapping )
            {
                CAFFA_ERROR( "Found label " << entryLabel << "(" << static_cast<int>( entry ) << ")" );
            }
            throw std::runtime_error( "AppEnum does not have the value " + std::to_string( static_cast<int>( enumValue ) ) );
        }

        return label;
    }

    size_t index( Enum enumValue ) const
    {
        std::optional<size_t> foundIndex;
        for ( size_t i = 0; i < m_mapping.size(); ++i )
        {
            if ( m_mapping[i].first == enumValue )
            {
                foundIndex = i;
                break;
            }
        }

        if ( !foundIndex.has_value() )
        {
            CAFFA_ERROR( "No value " << static_cast<int>( value ) << " in AppEnum" );
            for ( const auto& [entry, label] : m_mapping )
            {
                CAFFA_ERROR( "Found label " << label << "(" << static_cast<int>( entry ) << ")" );
            }

            throw std::runtime_error( "AppEnum does not have the value " + std::to_string( static_cast<int>( enumValue ) ) );
        }
        return *foundIndex;
    }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const std::string& label ) { return AppEnum<Enum>().enumVal( label ).has_value(); }
    static bool   isValid( size_t index ) { return AppEnum<Enum>().enumval( index ).has_value(); }
    static size_t validSize() { return AppEnum<Enum>().size(); }

    static std::vector<std::string> validLabels() { return AppEnum<Enum>().labels(); }

    static size_t      getIndex( Enum enumValue ) { return AppEnum<Enum>().index( enumValue ); }
    static std::string getLabel( Enum enumValue ) { return AppEnum<Enum>().label( enumValue ); }

private:
    //==================================================================================================
    /// The setup method is supposed to be specialized for each and every type instantiation of this class,
    /// and is supposed to set up the mapping between enum values, label and ui-label using the \m addItem
    /// method. It may also set a default value using \m setDefault
    //==================================================================================================
    void setUp();
    void addItem( Enum enumVal, const std::string& label ) { m_mapping.push_back( std::make_pair( enumVal, label ) ); }

    void setDefault( Enum defaultEnumValue ) { m_defaultValue = defaultEnumValue; }

    std::optional<Enum> m_value;
    std::optional<Enum> m_defaultValue;

    std::vector<std::pair<Enum, std::string>> m_mapping;
};

template <typename EnumType>
struct PortableDataType<AppEnum<EnumType>>
{
    static std::string name()
    {
        auto              labels = AppEnum<EnumType>::validLabels();
        std::stringstream ss;
        ss << "AppEnum(";
        for ( size_t i = 0; i < labels.size(); ++i )
        {
            if ( i > 0u ) ss << ",";
            ss << labels[i];
        }
        ss << ")";
        return ss.str();
    }
    static nlohmann::json jsonType()
    {
        auto values = nlohmann::json::array();
        for ( auto entry : AppEnum<EnumType>::validLabels() )
        {
            values.push_back( entry );
        }
        auto object    = nlohmann::json::object();
        object["enum"] = values;
        return object;
    }
};
//==================================================================================================
/// Implementation of stream operators to make Field<AppEnum<> > work smoothly
/// Assumes that the stream ends at the end of the enum label
//==================================================================================================

template <typename Enum>
std::istream& operator>>( std::istream& str, caffa::AppEnum<Enum>& appEnum )
{
    std::string label;
    str >> label;

    appEnum.setFromLabel( label );

    return str;
}

template <typename Enum>
std::ostream& operator<<( std::ostream& str, const caffa::AppEnum<Enum>& appEnum )
{
    auto value = appEnum.value();
    str << appEnum.label( value );
    return str;
}

template <typename Enum>
void to_json( nlohmann::json& jsonValue, const AppEnum<Enum>& appEnum )
{
    std::stringstream stream;
    stream << appEnum;
    jsonValue = stream.str();
}

template <typename Enum>
void from_json( const nlohmann::json& jsonValue, AppEnum<Enum>& appEnum )
{
    std::stringstream stream( jsonValue.get<std::string>() );
    stream >> appEnum;
}

} // namespace caffa
