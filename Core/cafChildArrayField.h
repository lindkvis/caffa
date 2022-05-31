#pragma once

#include "cafChildArrayFieldAccessor.h"
#include "cafChildArrayFieldHandle.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObservingPointer.h"
#include "cafPortableDataType.h"

#include <memory>

namespace caffa
{
template <typename T>
class FieldJsonCap;

/// \private
template <typename DataType>
class ChildArrayField : public ChildArrayFieldHandle
{
public:
    ChildArrayField()
    {
        bool doNotUsePointersFieldForAnythingButPointersToObject = false;
        CAFFA_ASSERT( doNotUsePointersFieldForAnythingButPointersToObject );
    }
};

/**
 * @brief FieldClass to handle a collection of Object derived pointers
 * The ChildArrayField will take over ownership of any object assigned to it.
 */
template <typename DataType>
class ChildArrayField<DataType*> : public ChildArrayFieldHandle
{
    typedef std::unique_ptr<DataType> DataTypeUniquePtr;

public:
    using FieldDataType         = DataType*;
    using DataAccessor          = ChildArrayFieldAccessor;
    using DirectStorageAccessor = ChildArrayFieldDirectStorageAccessor;

    ChildArrayField()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>( this ) )
    {
    }
    ~ChildArrayField() override;

    ChildArrayField&       operator()() { return *this; }
    const ChildArrayField& operator()() const { return *this; }

    // Reimplementation of PointersFieldHandle methods

    size_t                                     size() const override { return m_fieldDataAccessor->size(); }
    std::vector<std::unique_ptr<ObjectHandle>> clear() override;
    ObjectHandle*                              at( size_t index ) override;
    std::vector<DataType*>                     value() const;
    void                                       setValue( std::vector<std::unique_ptr<DataType>>& objects );

    // std::vector-like access

    DataType* operator[]( size_t index ) const;

    void push_back( DataTypeUniquePtr pointer );
    void push_back_obj( std::unique_ptr<ObjectHandle> obj ) override;
    void insert( size_t index, DataTypeUniquePtr pointer );
    void insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) override;
    void erase( size_t index ) override;

    // Child objects
    std::vector<ObjectHandle*>    childObjects() const override;
    std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;

    std::string dataType() const override { return std::string( "object[]" ); }

    void setAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

private: // To be disabled
    ChildArrayField( const ChildArrayField& ) = delete;
    ChildArrayField& operator=( const ChildArrayField& ) = delete;

private:
    friend class FieldJsonCap<ChildArrayField<DataType*>>;
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildArrayField.inl"
