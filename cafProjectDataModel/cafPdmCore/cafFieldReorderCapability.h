//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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

#include "cafFieldCapability.h"
#include "cafPtrArrayFieldHandle.h"
#include "cafSignal.h"

namespace caf
{
class ObjectHandle;

class FieldReorderCapability : public FieldCapability, public SignalEmitter, public SignalObserver
{
public:
    Signal<> orderChanged;

public:
    FieldReorderCapability( PtrArrayFieldHandle* field, bool giveOwnership );

    bool canItemBeMovedUp( size_t index ) const;
    bool canItemBeMovedDown( size_t index ) const;

    bool moveItemToTop( size_t index );
    bool moveItemUp( size_t index );
    bool moveItemDown( size_t index );

    static FieldReorderCapability* addToField( PtrArrayFieldHandle* field );
    template <typename ObserverClassType, typename... Args>
    static FieldReorderCapability*
        addToFieldWithCallback( PtrArrayFieldHandle* field,
                                ObserverClassType*      observer,
                                void ( ObserverClassType::*method )( const SignalEmitter*, Args... args ) )
    {
        FieldReorderCapability* reorderCapability = addToField( field );
        reorderCapability->orderChanged.connect( observer, method );
        return reorderCapability;
    }
    static bool fieldIsReorderable( PtrArrayFieldHandle* field );

    static FieldReorderCapability* reorderCapabilityOfParentContainer( const ObjectHandle* pdmObject );

    void onMoveItemToTop( const SignalEmitter* emitter, size_t index );
    void onMoveItemUp( const SignalEmitter* emitter, size_t index );
    void onMoveItemDown( const SignalEmitter* emitter, size_t index );

private:
    PtrArrayFieldHandle* m_field;
};

}; // namespace caf
