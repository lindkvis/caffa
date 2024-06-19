// ##################################################################################################
//
//    Caffa
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
#pragma once

#include <memory>
#include <vector>

namespace caffa
{
class FieldHandle;
class ObjectHandle;

class ChildArrayFieldAccessor
{
public:
    ChildArrayFieldAccessor( FieldHandle* field )
        : m_field( field )
    {
    }
    virtual ~ChildArrayFieldAccessor()                                       = default;
    virtual size_t                                           size() const    = 0;
    virtual void                                             clear()         = 0;
    virtual std::vector<std::shared_ptr<ObjectHandle>>       objects()       = 0;
    virtual std::vector<std::shared_ptr<const ObjectHandle>> objects() const = 0;

    virtual std::shared_ptr<ObjectHandle> at( size_t index ) const                                      = 0;
    virtual void                          insert( size_t index, std::shared_ptr<ObjectHandle> pointer ) = 0;
    virtual void                          push_back( std::shared_ptr<ObjectHandle> pointer )            = 0;
    virtual size_t                        index( std::shared_ptr<const ObjectHandle> pointer ) const    = 0;
    virtual void                          remove( size_t index )                                        = 0;

    /**
     * The accessor has a getter. Thus can be read.
     * @return true if it has a getter
     */
    virtual bool hasSetter() const = 0;

    /**
     * The accessor has a setter. Thus can be written to.
     * @return true if it has a setter
     */
    virtual bool hasGetter() const = 0;

protected:
    FieldHandle* m_field;
};

class ChildArrayFieldDirectStorageAccessor : public ChildArrayFieldAccessor
{
public:
    ChildArrayFieldDirectStorageAccessor( FieldHandle* field );
    ~ChildArrayFieldDirectStorageAccessor() override;

    size_t                                           size() const override;
    void                                             clear() override;
    std::vector<std::shared_ptr<ObjectHandle>>       objects() override;
    std::vector<std::shared_ptr<const ObjectHandle>> objects() const override;
    std::shared_ptr<ObjectHandle>                    at( size_t index ) const override;
    void         insert( size_t index, std::shared_ptr<ObjectHandle> pointer ) override;
    void         push_back( std::shared_ptr<ObjectHandle> pointer ) override;
    size_t       index( std::shared_ptr<const ObjectHandle> object ) const override;
    virtual void remove( size_t index ) override;
    bool         hasGetter() const override { return true; }
    bool         hasSetter() const override { return true; }

private:
    std::vector<std::shared_ptr<ObjectHandle>> m_pointers;
};

} // namespace caffa
