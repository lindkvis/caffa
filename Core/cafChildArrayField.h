#pragma once

#include "cafAssert.h"
#include "cafChildArrayFieldAccessor.h"
#include "cafChildArrayFieldHandle.h"
#include "cafFieldHandle.h"
#include "cafObjectPortableDataType.h"

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

    using Ptr           = std::shared_ptr<DataType>;
    using ConstPtr      = std::shared_ptr<const DataType>;
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
    operator std::vector<std::shared_ptr<DataType>>() { return this->objects(); }
    operator std::vector<std::shared_ptr<const DataType>>() const { return this->objects(); }

    size_t                                       size() const override { return m_fieldDataAccessor->size(); }
    void                                         clear() override;
    ObjectHandle::Ptr                            at( size_t index ) override;
    std::vector<std::shared_ptr<DataType>>       objects();
    std::vector<std::shared_ptr<const DataType>> objects() const;
    void                                         setObjects( std::vector<std::shared_ptr<DataType>>& objects );

    // std::vector-like access

    std::shared_ptr<DataType> operator[]( size_t index ) const;

    void push_back( std::shared_ptr<DataType> pointer );
    void push_back_obj( ObjectHandle::Ptr obj ) override;
    void insert( size_t index, std::shared_ptr<DataType> pointer );
    void insertAt( size_t index, ObjectHandle::Ptr obj ) override;
    void erase( size_t index ) override;

    // Child objects
    std::vector<ObjectHandle::Ptr>      childObjects() override;
    std::vector<ObjectHandle::ConstPtr> childObjects() const override;
    void                                removeChildObject( ObjectHandle::ConstPtr object );

    std::string dataType() const override { return PortableDataType<std::vector<DataType>>::name(); }

    bool isReadable() const override { return m_fieldDataAccessor != nullptr && m_fieldDataAccessor->hasGetter(); }
    bool isWritable() const override { return m_fieldDataAccessor != nullptr && m_fieldDataAccessor->hasSetter(); }

    void setAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    virtual std::string childClassKeyword() const override { return std::string( DataType::classKeywordStatic() ); }

private: // To be disabled
    ChildArrayField( const ChildArrayField& )            = delete;
    ChildArrayField& operator=( const ChildArrayField& ) = delete;

private:
    friend class FieldJsonCap<ChildArrayField<DataType*>>;
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildArrayField.inl"
