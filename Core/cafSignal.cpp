// ##################################################################################################
//
//    Custom Visualization Core library
//     Copyright (C) 2020-2023 Ceetron Solutions AS
//     Copyright (C) 2024- Kontur AS
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
#include "cafSignal.h"

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SignalEmitter::SignalEmitter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SignalEmitter::~SignalEmitter() noexcept
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SignalEmitter::addEmittedSignal( AbstractSignal* signalToAdd ) const
{
    m_signals.insert( signalToAdd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<AbstractSignal*> SignalEmitter::emittedSignals() const
{
    return m_signals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SignalObserver::SignalObserver()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SignalObserver::~SignalObserver() noexcept
{
    disconnectAllSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<AbstractSignal*> SignalObserver::observedSignals() const
{
    return m_signals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SignalObserver::addObservedSignal( AbstractSignal* signalToObserve ) const
{
    m_signals.insert( signalToObserve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SignalObserver::removeObservedSignal( AbstractSignal* signalToRemove ) const noexcept
{
    // This does not throw, since std::set::erase only throws if the comparison operator throws
    // and we're just comparing pointers.
    m_signals.erase( signalToRemove );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SignalObserver::disconnectAllSignals() noexcept
{
    auto observedSignals = m_signals;
    for ( auto observedSignal : observedSignals )
    {
        observedSignal->disconnect( const_cast<SignalObserver*>( this ) );
    }
}
