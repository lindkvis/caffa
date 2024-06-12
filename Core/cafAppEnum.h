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
#include <list>
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

    AppEnum() { m_value = EnumMapper::instance()->defaultValue(); }
    AppEnum( Enum value )
        : m_value( value )
    {
    }
    AppEnum( const std::string& value )
    {
        m_value = EnumMapper::instance()->defaultValue();
        setFromLabel( value );
    }

    auto operator<=>( const AppEnum& rhs ) const = default;

    Enum        value() const { return m_value; }
    size_t      index() const { return EnumMapper::instance()->index( m_value ); }
    std::string label() const { return EnumMapper::instance()->label( m_value ); }

    AppEnum& operator=( Enum value )
    {
        m_value = value;
        return *this;
    }

    void setFromLabel( const std::string& label )
    {
        if ( !EnumMapper::instance()->enumVal( m_value, label ) )
        {
            throw std::runtime_error( label + " is not a valid option" );
        }
    }

    void setFromIndex( size_t index )
    {
        if ( !EnumMapper::instance()->enumVal( m_value, index ) )
        {
            throw std::runtime_error( std::to_string( index ) + " is not a valid option index" );
        }
    }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const std::string& label ) { return EnumMapper::instance()->isValid( label ); }
    static bool   isValid( size_t index ) { return index < EnumMapper::instance()->size(); }
    static size_t size() { return EnumMapper::instance()->size(); }

    static std::vector<std::string> labels() { return EnumMapper::instance()->labels(); }
    static auto                     fromIndex( size_t idx )
    {
        Enum val;
        if ( !EnumMapper::instance()->enumVal( val, idx ) )
        {
            throw std::runtime_error( std::to_string( idx ) + " is not a valid option index" );
        }
        return AppEnum<Enum>( val );
    }
    static auto        fromLabel( const std::string& label ) { return AppEnum<Enum>( label ); }
    static size_t      index( Enum enumValue ) { return EnumMapper::instance()->index( enumValue ); }
    static std::string label( Enum enumValue ) { return EnumMapper::instance()->label( enumValue ); }
    static std::string labelFromIndex( size_t idx ) { return label( fromIndex( idx ).value() ); }

private:
    //==================================================================================================
    /// The setup method is supposed to be specialized for each and every type instantiation of this class,
    /// and is supposed to set up the mapping between enum values, label and ui-label using the \m addItem
    /// method. It may also set a default value using \m setDefault
    //==================================================================================================
    static void setUp();
    static void addItem( Enum enumVal, const std::string& label ) { EnumMapper::instance()->addItem( enumVal, label ); }

    static void setDefault( Enum defaultEnumValue ) { EnumMapper::instance()->setDefault( defaultEnumValue ); }

    Enum m_value;

    //==================================================================================================
    /// A private class to handle the instance of the mapping vector.
    /// all access methods could have been placed directly in the \class AppEnum class,
    /// but AppEnum implementation gets nicer this way.
    /// The real core of this class is the vector map member and the static instance method
    //==================================================================================================

    class EnumMapper
    {
    public:
        void addItem( Enum enumVal, const std::string& label )
        {
            instance()->m_mapping.push_back( std::make_pair( enumVal, label ) );
        }

        static EnumMapper* instance()
        {
            static EnumMapper storedInstance;
            static bool       isInitialized = false;
            if ( !isInitialized )
            {
                isInitialized = true;
                AppEnum<Enum>::setUp();
            }
            return &storedInstance;
        }

        void setDefault( Enum defaultEnumValue )
        {
            m_defaultValue      = defaultEnumValue;
            m_defaultValueIsSet = true;
        }

        Enum defaultValue() const
        {
            if ( m_defaultValueIsSet )
            {
                return m_defaultValue;
            }
            else
            {
                CAFFA_ASSERT( m_mapping.size() );
                return m_mapping[0].first;
            }
        }

        bool isValid( const std::string& label ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( label == m_mapping[idx].second ) return true;
            }

            return false;
        }

        size_t size() const { return m_mapping.size(); }

        bool enumVal( Enum& value, const std::string& label ) const
        {
            value = defaultValue();
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( label == m_mapping[idx].second )
                {
                    value = m_mapping[idx].first;
                    return true;
                }
            }
            return false;
        }

        bool enumVal( Enum& value, size_t index ) const
        {
            value = defaultValue();
            if ( index < m_mapping.size() )
            {
                value = m_mapping[index].first;
                return true;
            }
            else
                return false;
        }

        size_t index( Enum value ) const
        {
            for ( size_t i = 0; i < m_mapping.size(); ++i )
            {
                if ( value == m_mapping[i].first ) return i;
            }

            CAFFA_ERROR( "No value " << static_cast<int>( value ) << " in AppEnum" );
            for ( const auto& [enumVal, label] : m_mapping )
            {
                CAFFA_ERROR( "Found label " << label << "(" << static_cast<int>( enumVal ) << ")" );
            }

            throw std::runtime_error( "AppEnum does not have the value " + std::to_string( static_cast<int>( value ) ) );
        }

        std::string label( Enum value ) const
        {
            for ( const auto& [enumVal, label] : m_mapping )
            {
                if ( value == enumVal ) return label;
            }

            CAFFA_ERROR( "No value " << static_cast<int>( value ) << " in AppEnum" );
            for ( const auto& [enumVal, label] : m_mapping )
            {
                CAFFA_ERROR( "Found label " << label << "(" << static_cast<int>( enumVal ) << ")" );
            }
            throw std::runtime_error( "AppEnum does not have the value " + std::to_string( static_cast<int>( value ) ) );
        }

        std::vector<std::string> labels() const
        {
            std::vector<std::string> labelList;
            for ( const auto& [ignore, label] : m_mapping )
            {
                labelList.push_back( label );
            }
            return labelList;
        }

    private:
        EnumMapper()
            : m_defaultValueIsSet( false )
        {
        }

        std::vector<std::pair<Enum, std::string>> m_mapping;
        Enum                                      m_defaultValue;
        bool                                      m_defaultValueIsSet;
    };
};

template <typename EnumType>
struct PortableDataType<AppEnum<EnumType>>
{
    static std::string name()
    {
        auto              labels = AppEnum<EnumType>::labels();
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
        for ( auto entry : AppEnum<EnumType>::labels() )
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
    std::string label = appEnum.label();
    str << appEnum.label();
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
