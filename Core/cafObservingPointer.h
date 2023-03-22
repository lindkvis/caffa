// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2011-2013 Ceetron AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
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

#include <cstddef>

namespace caffa
{
class ObjectHandle;

//==================================================================================================
/// Helper class for the Pointer class
/// The add and removing of references is put into a pure static class to
/// resolve circular include problems.
//
/// Overall idea of the referencing system:
/// The addressToObjectPointer is added to a std::set in the object pointed to.
/// when the object pointed to is deleted, its destructor sets the object pointers
/// it has addresses to nullptr
//==================================================================================================

class ObservingPointerImpl
{
private:
    template <class T>
    friend class ObservingPointer;
    static void addReference( ObjectHandle** addressToObjectPointer );
    static void removeReference( ObjectHandle** addressToObjectPointer );
};

//==================================================================================================
/// Guarded pointer class to point at Objects
/// Use a Pointer<SomeObject> in the same way as a normal pointer.
/// The guarding sets the pointer to nullptr if the object pointed to dies
///
//==================================================================================================

template <class T>
class ObservingPointer
{
    ObjectHandle* m_object;

public:
    inline ObservingPointer()
        : m_object( nullptr )
    {
    }
    inline ObservingPointer( T* p )
        : m_object( p )
    {
        ObservingPointerImpl::addReference( &m_object );
    }
    inline ObservingPointer( const ObservingPointer<T>& p )
        : m_object( p.m_object )
    {
        ObservingPointerImpl::addReference( &m_object );
    }
    inline ~ObservingPointer() { ObservingPointerImpl::removeReference( &m_object ); }

    T*                   p() const { return static_cast<T*>( const_cast<ObjectHandle*>( m_object ) ); }
    bool                 isNull() const { return !m_object; }
    bool                 notNull() const { return !isNull(); }
                         operator T*() const { return static_cast<T*>( const_cast<ObjectHandle*>( m_object ) ); }
    T&                   operator*() const { return *static_cast<T*>( const_cast<ObjectHandle*>( m_object ) ); }
    T*                   operator->() const { return static_cast<T*>( const_cast<ObjectHandle*>( m_object ) ); }
    ObservingPointer<T>& operator=( const ObservingPointer<T>& p )
    {
        if ( this != &p ) ObservingPointerImpl::removeReference( &m_object );
        m_object = p.m_object;
        ObservingPointerImpl::addReference( &m_object );
        return *this;
    }
    ObservingPointer<T>& operator=( T* p )
    {
        if ( m_object != p ) ObservingPointerImpl::removeReference( &m_object );
        m_object = p;
        ObservingPointerImpl::addReference( &m_object );
        return *this;
    }
    template <class S>
    auto operator<=>( const ObservingPointer<S>& rhs ) const
    {
        return m_object <=> rhs.rawPtr();
    }

    // Private methods used by Field<T*> and PointersField<T*>. Do not use unless you mean it !
    ObjectHandle* rawPtr() const { return m_object; }
    void          setRawPtr( ObjectHandle* p )
    {
        if ( m_object != p ) ObservingPointerImpl::removeReference( &m_object );
        m_object = p;
        ObservingPointerImpl::addReference( &m_object );
    }
};

} // End of namespace caffa
