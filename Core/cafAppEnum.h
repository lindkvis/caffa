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

#include "cafPortableDataType.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

namespace caffa
{
//==================================================================================================
/// An enum class to make it easier to handle IO and UI based on the enum.
/// Usage:
/// In Header file of SomeClass:
///    enum SomeEnumType
///    {
///       A = 2,
///       B = 7
///    };
///   caffa::AppEnum<SomeEnumType> m_enumValue;
///
/// In C++ file :
///    namespace caffa {
///    template<>
///    void caffa::AppEnum<SomeClass::SomeEnumType>::setUp()
///    {
///        addItem(SomeClass::A,           "A");
///        addItem(SomeClass::B,           "B");
///        setDefault(SomeClass::B);
///    }
///    }
/// General use:
///
///    m_enumValue = A;
///    if (m_enumValue == A || m_enumValue != B ){}
///
///    switch (m_enumValue)
///    {
///    case A:
///        break;
///    case B:
///        break;
///    }
///
///    cout << m_enumValue.label();
///    m_enumValue.setFromLabel("A");
///
///    for (size_t i = 0; i < caffa::AppEnum<SomeClass::SomeEnumType>::size(); ++i)
///        cout << caffa::AppEnum<SomeClass::SomeEnumType>::label(caffa::AppEnum<SomeClass::SomeEnumType>::fromIndex(i))
///        << endl;
///
///
//==================================================================================================

template <class T>
class AppEnum
{
public:
    using DataType = T;

    AppEnum() { m_value = EnumMapper::instance()->defaultValue(); }
    AppEnum( T value )
        : m_value( value )
    {
    }
    AppEnum( const std::string& value )
    {
        m_value = EnumMapper::instance()->defaultValue();
        setFromLabel( value );
    }

    bool operator==( T value ) const { return m_value == value; }
    bool operator!=( T value ) const { return m_value != value; }

    T           value() const { return m_value; }
    size_t      index() const { return EnumMapper::instance()->index( m_value ); }
    std::string label() const { return EnumMapper::instance()->label( m_value ); }

    AppEnum& operator=( T value )
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
    static AppEnum<T>               fromIndex( size_t idx )
    {
        T val;
        if ( !EnumMapper::instance()->enumVal( val, idx ) )
        {
            throw std::runtime_error( std::to_string( idx ) + " is not a valid option index" );
        }
        return AppEnum<T>( val );
    }
    static AppEnum<T>  fromLabel( const std::string& label ) { return AppEnum<T>( label ); }
    static size_t      index( T enumValue ) { return EnumMapper::instance()->index( enumValue ); }
    static std::string label( T enumValue ) { return EnumMapper::instance()->label( enumValue ); }
    static std::string labelFromIndex( size_t idx ) { return label( fromIndex( idx ).value() ); }

private:
    //==================================================================================================
    /// The setup method is supposed to be specialized for each and every type instantiation of this class,
    /// and is supposed to set up the mapping between enum values, label and ui-label using the \m addItem
    /// method. It may also set a default value using \m setDefault
    //==================================================================================================
    static void setUp();
    static void addItem( T enumVal, const std::string& label ) { EnumMapper::instance()->addItem( enumVal, label ); }

    static void setDefault( T defaultEnumValue ) { EnumMapper::instance()->setDefault( defaultEnumValue ); }

    T m_value;

    //==================================================================================================
    /// A private class to handle the instance of the mapping vector.
    /// all access methods could have been placed directly in the \class AppEnum class,
    /// but AppEnum implementation gets nicer this way.
    /// The real core of this class is the vector map member and the static instance method
    //==================================================================================================

    class EnumMapper
    {
    private:
        struct Entry
        {
            Entry( T enumVal, const std::string& label )
                : m_enumVal( enumVal )
                , m_label( label )
            {
            }

            T           m_enumVal;
            std::string m_label;
        };

    public:
        void addItem( T enumVal, const std::string& label )
        {
            instance()->m_mapping.push_back( Entry( enumVal, label ) );
        }

        static EnumMapper* instance()
        {
            static EnumMapper storedInstance;
            static bool       isInitialized = false;
            if ( !isInitialized )
            {
                isInitialized = true;
                AppEnum<T>::setUp();
            }
            return &storedInstance;
        }

        void setDefault( T defaultEnumValue )
        {
            m_defaultValue      = defaultEnumValue;
            m_defaultValueIsSet = true;
        }

        T defaultValue() const
        {
            if ( m_defaultValueIsSet )
            {
                return m_defaultValue;
            }
            else
            {
                // CAFFA_ASSERT(m_mapping.size());
                return m_mapping[0].m_enumVal;
            }
        }

        bool isValid( const std::string& label ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( label == m_mapping[idx].m_label ) return true;
            }

            return false;
        }

        size_t size() const { return m_mapping.size(); }

        bool enumVal( T& value, const std::string& label ) const
        {
            value = defaultValue();
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( label == m_mapping[idx].m_label )
                {
                    value = m_mapping[idx].m_enumVal;
                    return true;
                }
            }
            return false;
        }

        bool enumVal( T& value, size_t index ) const
        {
            value = defaultValue();
            if ( index < m_mapping.size() )
            {
                value = m_mapping[index].m_enumVal;
                return true;
            }
            else
                return false;
        }

        size_t index( T enumValue ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( enumValue == m_mapping[idx].m_enumVal ) return idx;
            }

            return idx;
        }

        std::vector<std::string> labels() const
        {
            std::vector<std::string> labelList;
            size_t                   idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                labelList.push_back( m_mapping[idx].m_label );
            }
            return labelList;
        }

        std::string label( T value ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_label;
            }
            return "";
        }

    private:
        EnumMapper()
            : m_defaultValueIsSet( false )
        {
        }

        friend class AppEnum<T>;

        std::vector<Entry> m_mapping;
        T                  m_defaultValue;
        bool               m_defaultValueIsSet;
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
};

template <typename EnumType>
struct PortableDataType<std::vector<AppEnum<EnumType>>>
{
    static std::string name()
    {
        auto              labels = AppEnum<EnumType>::labels();
        std::stringstream ss;
        ss << "AppEnum[](";
        for ( size_t i = 0; i < labels.size(); ++i )
        {
            if ( i > 0u ) ss << ",";
            ss << labels[i];
        }
        ss << ")";
        return ss.str();
    }
};

template <typename EnumType>
struct PortableDataType<std::vector<std::vector<AppEnum<EnumType>>>>
{
    static std::string name()
    {
        auto              labels = AppEnum<EnumType>::labels();
        std::stringstream ss;
        ss << "AppEnum[][](";
        for ( size_t i = 0; i < labels.size(); ++i )
        {
            if ( i > 0u ) ss << ",";
            ss << labels[i];
        }
        ss << ")";
        return ss.str();
    }
};

//==================================================================================================
/// Implementation of stream operators to make Field<AppEnum<> > work smoothly
/// Assumes that the stream ends at the end of the enum label
//==================================================================================================

template <typename T>
std::istream& operator>>( std::istream& str, caffa::AppEnum<T>& appEnum )
{
    std::string label;
    str >> label;

    appEnum.setFromLabel( label );

    return str;
}

template <typename T>
std::ostream& operator<<( std::ostream& str, const caffa::AppEnum<T>& appEnum )
{
    std::string label = appEnum.label();
    str << appEnum.label();
    return str;
}

template <typename T>
void to_json( nlohmann::json& jsonValue, const AppEnum<T>& appEnum )
{
    std::stringstream stream;
    stream << appEnum;
    jsonValue = stream.str();
}

template <typename T>
void from_json( const nlohmann::json& jsonValue, AppEnum<T>& appEnum )
{
    std::stringstream stream( jsonValue.get<std::string>() );
    stream >> appEnum;
}

} // namespace caffa

//==================================================================================================
/// Cant remember why we need those comparison operators...
//==================================================================================================

template <class T>
bool operator==( T value, const caffa::AppEnum<T>& appEnum )
{
    return ( appEnum == value );
}

template <class T>
bool operator!=( T value, const caffa::AppEnum<T>& appEnum )
{
    return ( appEnum != value );
}
