#pragma once

#include "cafChildArrayFieldAccessor.h"
#include "cafChildArrayFieldHandle.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafPointer.h"
#include "cafPortableDataType.h"

#include <memory>

namespace caffa
{
template <typename T>
class FieldIoCap;

//==================================================================================================
/// FieldClass to handle a collection of Object derived pointers
/// The reasons for this class is to add itself as parentField into the objects being pointed to.
/// The interface is made similar to std::vector<>, and the complexity of the methods is similar too.
//==================================================================================================

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
    void push_back_obj( std::unique_ptr<ObjectHandle> obj );
    void insert( size_t index, DataTypeUniquePtr pointer );
    void insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) override;
    void erase( size_t index ) override;

    // Child objects
    std::vector<ObjectHandle*>    childObjects() const override;
    void                          childObjects( std::vector<ObjectHandle*>* objects ) const override;
    std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;

    std::string dataType() const override { return std::string( "object[]" ); }

private: // To be disabled
    CAFFA_DISABLE_COPY_AND_ASSIGN( ChildArrayField );

private:
    friend class FieldIoCap<ChildArrayField<DataType*>>;
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildArrayField.inl"
