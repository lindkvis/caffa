// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) 2020-2023 Ceetron Solutions AS
//    Copyright (C) 2024- Kontur AS
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

#include "cafAssert.h"

#include <functional>
#include <map>
#include <set>
#include <string>
#include <type_traits>

namespace caffa
{
class SignalEmitter;
class SignalObserver;

class AbstractSignal
{
public:
    virtual ~AbstractSignal() noexcept                  = default;
    virtual void disconnect( SignalObserver* observer ) = 0;
};

//==================================================================================================
/// SignalEmitter
/// Should be inherited by any object that emits signals.
//==================================================================================================
class SignalEmitter
{
public:
    SignalEmitter();
    virtual ~SignalEmitter() noexcept;

    void                      addEmittedSignal( AbstractSignal* signalToAdd ) const;
    std::set<AbstractSignal*> emittedSignals() const;

private:
    mutable std::set<AbstractSignal*> m_signals;
};

//==================================================================================================
/// SignalObserver.
/// Should be inherited by any object that observes and reacts to signals.
//==================================================================================================
class SignalObserver
{
public:
    SignalObserver();
    virtual ~SignalObserver() noexcept;
    std::set<AbstractSignal*> observedSignals() const;
    void                      addObservedSignal( AbstractSignal* signalToAdd ) const;
    void                      removeObservedSignal( AbstractSignal* signalToRemove ) const noexcept;

private:
    void disconnectAllSignals() noexcept;

private:
    mutable std::set<AbstractSignal*> m_signals;
};

//==================================================================================================
/// General signal class.
/// Connect any member function with the signature void(const Signal*, const SignalData* data)
/// Connect with .connect(this, &Class::nameOfMethod)
/// The method should accept that data may be nullptr
//==================================================================================================
template <typename... Args>
class Signal : public AbstractSignal
{
public:
    using MemberCallback = std::function<void( const SignalEmitter*, Args... args )>;

public:
    Signal( const SignalEmitter* emitter )
        : m_emitter( emitter )
    {
        m_emitter->addEmittedSignal( this );
    }

    virtual ~Signal()
    {
        for ( auto& [observer, callback] : m_observerCallbacks )
        {
            observer->removeObservedSignal( this );
        }
    }

    template <typename ClassType>
    void connect( ClassType* observer, void ( ClassType::*method )( const SignalEmitter*, Args... args ) )
    {
        MemberCallback lambda = [=]( const SignalEmitter* emitter, Args... args )
        {
            // Call method
            ( observer->*method )( emitter, args... );
        };
        connect( observer, lambda );
    }

    template <typename ClassType>
    void connect( ClassType* observer, const MemberCallback& callback )
    {
        static_assert( std::is_convertible<ClassType*, SignalObserver*>::value,
                       "Only classes that inherit SignalObserver can connect as an observer of a Signal." );
        m_observerCallbacks[observer] = callback;
        observer->addObservedSignal( this );
    }

    // Disconnect an observer from the signal. Do this only when the relationship between the
    // observer and emitter is severed but the object kept alive.
    // There's no need to do this when deleting the observer.
    void disconnect( SignalObserver* observer ) noexcept override
    {
        // This does not throw, since std::map::erase only throws if the comparison operator throws
        // and we're just comparing pointers.
        m_observerCallbacks.erase( observer );
        observer->removeObservedSignal( this );
    }

    void send( Args... args ) const
    {
        auto observerCallBacksCopy = m_observerCallbacks;
        for ( const auto& [observer, callback] : observerCallBacksCopy )
        {
            callback( m_emitter, args... );
        }
    }

    size_t observerCount() const { return m_observerCallbacks.size(); }

    bool connected( const SignalObserver* observer ) const
    {
        // Possible to search for const-pointer due to transparent comparator
        auto it = m_observerCallbacks.find( observer );
        return it != m_observerCallbacks.end();
    }

private:
    Signal( const Signal& rhs )            = delete;
    Signal& operator=( const Signal& rhs ) = delete;

private:
    using TransparentComparator = std::less<>;
    std::map<SignalObserver*, MemberCallback, TransparentComparator> m_observerCallbacks;
    const SignalEmitter*                                             m_emitter;
};
} // namespace caffa
