#pragma once

#include "cafAssert.h"
#include "cafChildArrayFieldAccessor.h"
#include "cafChildArrayFieldHandle.h"
#include "cafFieldHandle.h"
#include "cafObservingPointer.h"
#include "cafPortableDataType.h"

#include <memory>

namespace caffa
{
class ObjectHandle;

template <typename T>
class FieldJsonCap;

/**
 * @brief Field class to handle a collection of Object derived pointers
 * The ChildArrayField will take over ownership of any object assigned to it.
 */
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
class ChildArrayField : public ChildArrayFieldHandle
{
public:
    using DataType = typename std::remove_pointer<DataTypePtr>::type;

    using UniquePtr     = std::unique_ptr<DataType>;
    using FieldDataType = DataTypePtr;

    using DataAccessor          = ChildArrayFieldAccessor;
    using DirectStorageAccessor = ChildArrayFieldDirectStorageAccessor;

    ChildArrayField()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>( this ) )
    {
        static_assert( std::is_base_of<ObjectHandle, DataType>::value &&
                       "Child Array fields can only contain ObjectHandle-derived objects" );
    }
    ~ChildArrayField() override;

    // Access operators
    operator std::vector<DataType*>() { return this->objects(); }
    operator std::vector<const DataType*>() const { return this->objects(); }

    ChildArrayField&       operator()() { return *this; }
    const ChildArrayField& operator()() const { return *this; }

    size_t                                     size() const override { return m_fieldDataAccessor->size(); }
    std::vector<std::unique_ptr<ObjectHandle>> clear() override;
    ObjectHandle*                              at( size_t index ) override;
    std::vector<DataType*>                     objects();
    std::vector<const DataType*>               objects() const;
    void                                       setObjects( std::vector<std::unique_ptr<DataType>>& objects );

    // std::vector-like access

    DataType* operator[]( size_t index ) const;

    void push_back( UniquePtr pointer );
    void push_back_obj( std::unique_ptr<ObjectHandle> obj ) override;
    void insert( size_t index, UniquePtr pointer );
    void insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) override;
    void erase( size_t index ) override;

    // Child objects
    std::vector<ObjectHandle*>       childObjects() override;
    std::vector<const ObjectHandle*> childObjects() const override;
    std::unique_ptr<ObjectHandle>    removeChildObject( ObjectHandle* object );

    std::string dataType() const override { return std::string( "object[]" ); }

    void setAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    virtual constexpr std::string_view childClassKeyword() const override { return DataType::classKeywordStatic(); }

private: // To be disabled
    ChildArrayField( const ChildArrayField& )            = delete;
    ChildArrayField& operator=( const ChildArrayField& ) = delete;

private:
    friend class FieldJsonCap<ChildArrayField<DataType*>>;
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildArrayField.inl"
