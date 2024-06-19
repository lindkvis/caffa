// ##################################################################################################
//
//    Caffa
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

#include "cafChildFieldHandle.h"

#include <memory>

namespace caffa
{
class ChildArrayFieldAccessor;
class ObjectHandle;

/**
 * @brief A non-templated base interface for ChildArrayField<DataType*>
 * Used so we can have pointers to any ChildArrayField.
 *
 */
class ChildArrayFieldHandle : public ChildFieldBaseHandle
{
public:
    ChildArrayFieldHandle() {}
    ~ChildArrayFieldHandle() override {}

    /**
     * @brief Get the number of child objects.
     *
     * @return size_t
     */
    virtual size_t size() const = 0;
    /**
     * @brief Check if the child array field is empty
     *
     * @return true if empty
     * @return false if there are child objects.
     */
    bool empty() const override { return this->size() == 0u; }

    /**
     * @brief Clear all content
     */
    virtual void clear() = 0;

    /**
     * @brief Erase a particular child object by index
     *
     * @param index the index to the object
     */
    virtual void erase( size_t index ) = 0;

    /**
     * @brief Get a raw pointer to the object at a particular index
     *
     * @param index The index to look up
     * @return A raw pointer to the Caffa object.
     */
    virtual std::shared_ptr<ObjectHandle> at( size_t index ) = 0;

    /**
     * @brief Insert an object at a particular index. Ownership will be taken.
     *
     * @param index the index to insert at
     * @param obj A unique pointer to the object.
     */
    virtual void insertAt( size_t index, std::shared_ptr<ObjectHandle> obj ) = 0;

    /**
     * @brief push back and object taking over ownership.
     *
     * @param obj object to take.
     */
    virtual void push_back_obj( std::shared_ptr<ObjectHandle> obj ) = 0;

    /**
     * @brief Set a new accessor
     *
     * @param accessor
     */
    virtual void setAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) = 0;
};

} // namespace caffa
