//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once

#include "cafPortableDataType.h"

#include <iostream>
#include <list>
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
///        addItem(SomeClass::A,           "A",         "An A letter");
///        addItem(SomeClass::B,           "B",         "A B letter");
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
///    cout << m_enumValue.text();
///    m_enumValue.setFromText("A");
///
///    for (size_t i = 0; i < caffa::AppEnum<SomeClass::SomeEnumType>::size(); ++i)
///        cout << caffa::AppEnum<SomeClass::SomeEnumType>::text(caffa::AppEnum<SomeClass::SomeEnumType>::fromIndex(i))
///        << endl;
///
///
///
///   Create a list of OptionItemInfos from AppEnum
///     QList<caffa::OptionItemInfo> options;
///     for (size_t i = 0; i < caffa::AppEnum<TestEnumType>::size(); ++i)
///     {
///         options.push_back(caffa::OptionItemInfo(caffa::AppEnum<TestEnumType>::uiTextFromIndex(i),
///         caffa::AppEnum<TestEnumType>::fromIndex(i)));
///     }
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

    bool operator==( T value ) const { return m_value == value; }
    bool operator!=( T value ) const { return m_value != value; }

    T           value() const { return m_value; }
    size_t      index() const { return EnumMapper::instance()->index( m_value ); }
    std::string text() const { return EnumMapper::instance()->text( m_value ); }
    std::string uiText() const { return EnumMapper::instance()->uiText( m_value ); }

    AppEnum& operator=( T value )
    {
        m_value = value;
        return *this;
    }
    bool setFromText( const std::string& text ) { return EnumMapper::instance()->enumVal( m_value, text ); }
    bool setFromIndex( size_t index ) { return EnumMapper::instance()->enumVal( m_value, index ); }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const std::string& text ) { return EnumMapper::instance()->isValid( text ); }
    static bool   isValid( size_t index ) { return index < EnumMapper::instance()->size(); }
    static size_t size() { return EnumMapper::instance()->size(); }

    static std::list<std::string> uiTexts() { return EnumMapper::instance()->uiTexts(); }
    static AppEnum<T>             fromIndex( size_t idx )
    {
        T val;
        EnumMapper::instance()->enumVal( val, idx );
        return AppEnum<T>( val );
    }
    static AppEnum<T> fromText( const std::string& text )
    {
        T val;
        EnumMapper::instance()->enumVal( val, text );
        return AppEnum<T>( val );
    }
    static size_t      index( T enumValue ) { return EnumMapper::instance()->index( enumValue ); }
    static std::string text( T enumValue ) { return EnumMapper::instance()->text( enumValue ); }
    static std::string textFromIndex( size_t idx ) { return text( fromIndex( idx ).value() ); }
    static std::string uiText( T enumValue ) { return EnumMapper::instance()->uiText( enumValue ); }
    static std::string uiTextFromIndex( size_t idx ) { return uiText( fromIndex( idx ).value() ); }

private:
    //==================================================================================================
    /// The setup method is supposed to be specialized for each and every type instantiation of this class,
    /// and is supposed to set up the mapping between enum values, text and ui-text using the \m addItem
    /// method. It may also set a default value using \m setDefault
    //==================================================================================================
    static void setUp();
    static void addItem( T enumVal, const std::string& text, const std::string& uiText )
    {
        EnumMapper::instance()->addItem( enumVal, text, uiText );
    }

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
        struct Triplet
        {
            Triplet( T enumVal, const std::string& text, std::string uiText )
                : m_enumVal( enumVal )
                , m_text( text )
                , m_uiText( uiText )
            {
            }

            T           m_enumVal;
            std::string m_text;
            std::string m_uiText;
        };

    public:
        void addItem( T enumVal, const std::string& text, std::string uiText )
        {
            instance()->m_mapping.push_back( Triplet( enumVal, text, uiText ) );
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

        bool isValid( const std::string& text ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( text == m_mapping[idx].m_text ) return true;
            }

            return false;
        }

        size_t size() const { return m_mapping.size(); }

        bool enumVal( T& value, const std::string& text ) const
        {
            value = defaultValue();
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( text == m_mapping[idx].m_text )
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

        std::string uiText( T value ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_uiText;
            }
            return "";
        }

        std::list<std::string> uiTexts() const
        {
            std::list<std::string> uiTextList;
            size_t                 idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                uiTextList.push_back( m_mapping[idx].m_uiText );
            }
            return uiTextList;
        }

        std::string text( T value ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_text;
            }
            return "";
        }

    private:
        EnumMapper()
            : m_defaultValueIsSet( false )
        {
        }

        friend class AppEnum<T>;

        std::vector<Triplet> m_mapping;
        T                    m_defaultValue;
        bool                 m_defaultValueIsSet;
    };
};

template <typename EnumType>
struct PortableDataType<AppEnum<EnumType>>
{
    static constexpr const char* name() { return "AppEnum"; }
};
template <typename EnumType>
struct PortableDataType<std::vector<AppEnum<EnumType>>>
{
    static constexpr const char* name() { return "vector:AppEnum"; }
};

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

//==================================================================================================
/// Implementation of stream operators to make Field<AppEnum<> > work smoothly
/// Assumes that the stream ends at the end of the enum text
//==================================================================================================

template <typename T>
std::istream& operator>>( std::istream& str, caffa::AppEnum<T>& appEnum )
{
    std::string text;
    str >> text;
    appEnum.setFromText( text );

    return str;
}

template <typename T>
std::ostream& operator<<( std::ostream& str, const caffa::AppEnum<T>& appEnum )
{
    std::string text = appEnum.text();
    str << text;
    return str;
}
