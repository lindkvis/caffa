#pragma once

#include "cafPtrArrayFieldHandle.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafPointer.h"

#include <memory>

namespace caf
{
template <typename T>
class FieldIoCap;

//==================================================================================================
///
///
///
//==================================================================================================
class ChildArrayFieldHandle : public PtrArrayFieldHandle
{
public:
    ChildArrayFieldHandle() {}
    ~ChildArrayFieldHandle() override {}

    virtual void                  clear()                                                     = 0;
    virtual Pointer<ObjectHandle> insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) = 0;

    bool hasSameFieldCountForAllObjects();
};

//==================================================================================================
/// FieldClass to handle a collection of Object derived pointers
/// The reasons for this class is to add itself as parentField into the objects being pointed to.
/// The interface is made similar to std::vector<>, and the complexity of the methods is similar too.
//==================================================================================================

template <typename DataType>
class ChildArrayField : public FieldHandle
{
public:
    ChildArrayField()
    {
        bool doNotUsePointersFieldForAnythingButPointersToObject = false;
        CAF_ASSERT( doNotUsePointersFieldForAnythingButPointersToObject );
    }
};

template <typename DataType>
class ChildArrayField<DataType*> : public ChildArrayFieldHandle
{
    typedef std::unique_ptr<DataType> DataTypeUniquePtr;

public:
    using FieldDataType = DataType*;

    ChildArrayField() {}
    ~ChildArrayField() override;

    ChildArrayField&       operator()() { return *this; }
    const ChildArrayField& operator()() const { return *this; }

    // Reimplementation of PointersFieldHandle methods

    size_t                                 size() const override { return m_pointers.size(); }
    bool                                   empty() const override { return m_pointers.empty(); }
    void                                   clear() override;
    std::vector<std::unique_ptr<DataType>> removeAll();
    ObjectHandle*                          at( size_t index ) override;
    void                                   setValue( const std::vector<std::unique_ptr<DataType>>& objects );

    // std::vector-like access

    DataType* operator[]( size_t index ) const;

    Pointer<DataType>     push_back( DataTypeUniquePtr pointer );
    Pointer<DataType>     insert( size_t index, DataTypeUniquePtr pointer );
    Pointer<ObjectHandle> insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) override;
    size_t                count( const DataType* pointer ) const;

    void   erase( size_t index ) override;
    size_t index( const DataType* pointer ) const;

    typename std::vector<Pointer<DataType>>::iterator begin() { return m_pointers.begin(); };
    typename std::vector<Pointer<DataType>>::iterator end() { return m_pointers.end(); };

    typename std::vector<Pointer<DataType>>::const_iterator begin() const { return m_pointers.begin(); };
    typename std::vector<Pointer<DataType>>::const_iterator end() const { return m_pointers.end(); };

    // Child objects
    std::vector<DataType*> childObjects() const;

    void                                        childObjects( std::vector<ObjectHandle*>* objects ) override;
    [[nodiscard]] std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;
    [[nodiscard]] std::unique_ptr<DataType>     remove( ObjectHandle* object );

private: // To be disabled
    CAF_DISABLE_COPY_AND_ASSIGN( ChildArrayField );

private:
    friend class FieldIoCap<ChildArrayField<DataType*>>;
    std::vector<Pointer<DataType>> m_pointers;
};

} // End of namespace caf

#include "cafChildArrayField.inl"
