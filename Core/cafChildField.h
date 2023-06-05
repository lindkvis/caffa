#pragma once

#include "cafAssert.h"
#include "cafChildFieldAccessor.h"
#include "cafChildFieldHandle.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafVisitor.h"

#include <concepts>
#include <memory>
#include <type_traits>

namespace caffa
{
template <typename T>
class FieldJsonCap;

/**
 * @brief Field class to handle a pointer to a caffa Object.
 *
 * @tparam DataTypePtr A pointer to a class derived from caffa::Object
 */
template <typename DataTypePtr>
    requires is_pointer<DataTypePtr>
class ChildField : public ChildFieldHandle
{
public:
    using DataType      = typename std::remove_pointer<DataTypePtr>::type;
    using Ptr           = std::shared_ptr<DataType>;
    using ConstPtr      = std::shared_ptr<const DataType>;
    using FieldDataType = DataTypePtr;

public:
    ChildField()
        : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>( this ) )
    {
        static_assert( DerivesFromObjectHandle<DataType> &&
                       "Child fields can only contain ObjectHandle-derived objects" );
    }

    virtual ~ChildField();

    // Assignment

    ChildField& operator=( Ptr object );
    bool        operator==( ObjectHandle::ConstPtr object ) const;
    bool        operator==( const ObjectHandle* object ) const;

    // Basic access

    std::shared_ptr<DataType> object() { return std::dynamic_pointer_cast<DataType>( m_fieldDataAccessor->object() ); }
    std::shared_ptr<const DataType> object() const
    {
        return std::dynamic_pointer_cast<const DataType>( m_fieldDataAccessor->object() );
    }
    void setObject( Ptr object );

    // Access operators
    operator std::shared_ptr<DataType>() { return this->object(); }
    operator std::shared_ptr<const DataType>() const { return this->object(); }

    // Deep copy of object content
    std::shared_ptr<DataType> deepCloneObject() const;
    void                      deepCopyObjectFrom( std::shared_ptr<const DataType> copyFrom );

    std::shared_ptr<DataType>       operator->() { return this->object(); }
    std::shared_ptr<const DataType> operator->() const { return this->object(); }

    std::shared_ptr<DataType>       operator()() { return this->object(); }
    std::shared_ptr<const DataType> operator()() const { return this->object(); }

    // Child objects
    std::vector<ObjectHandle::Ptr>      childObjects() override;
    std::vector<ObjectHandle::ConstPtr> childObjects() const override;
    void                                clear() override;
    void                                removeChildObject( ObjectHandle::ConstPtr object );
    void                                setChildObject( ObjectHandle::Ptr object );

    std::string dataType() const override { return PortableDataType<DataType>::name(); }

    void setAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    std::string childClassKeyword() const override { return std::string( DataType::classKeywordStatic() ); }

private:
    ChildField( const ChildField& )            = delete;
    ChildField& operator=( const ChildField& ) = delete;

    friend class FieldJsonCap<ChildField<DataType*>>;
    mutable std::unique_ptr<ChildFieldAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildField.inl"
