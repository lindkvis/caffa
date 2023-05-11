#pragma once

#include "cafAssert.h"
#include "cafChildFieldAccessor.h"
#include "cafChildFieldHandle.h"
#include "cafFieldHandle.h"
#include "cafObservingPointer.h"
#include "cafPortableDataType.h"
#include "cafVisitor.h"

#include <concepts>
#include <memory>
#include <type_traits>

namespace caffa
{
class ObjectHandle;

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
    using Ptr           = std::unique_ptr<DataType>;
    using FieldDataType = DataTypePtr;

public:
    ChildField()
        : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>( this ) )
    {
        static_assert( std::is_base_of<ObjectHandle, DataType>::value &&
                       "Child fields can only contain ObjectHandle-derived objects" );
    }

    virtual ~ChildField();

    // Assignment

    ChildField& operator=( Ptr object );

    // Basic access

    DataType*       object() { return static_cast<DataType*>( m_fieldDataAccessor->object() ); }
    const DataType* object() const { return static_cast<const DataType*>( m_fieldDataAccessor->object() ); }
    void            setObject( Ptr object );

    // Access operators
    operator DataType*() { return this->object(); }
    operator const DataType*() const { return this->object(); }

    // Deep copy of object content
    Ptr  deepCloneObject() const;
    void deepCopyObjectFrom( const DataType* copyFrom );

    DataType*       operator->() { return this->object(); }
    const DataType* operator->() const { return this->object(); }

    DataType*       operator()() { return static_cast<DataType*>( m_fieldDataAccessor->object() ); }
    const DataType* operator()() const { return static_cast<const DataType*>( m_fieldDataAccessor->object() ); }

    // Child objects
    std::vector<ObjectHandle*>       childObjects() override;
    std::vector<const ObjectHandle*> childObjects() const override;
    std::unique_ptr<ObjectHandle>    clear() override;
    std::unique_ptr<ObjectHandle>    removeChildObject( ObjectHandle* object );
    void                             setChildObject( std::unique_ptr<ObjectHandle> object );

    std::string dataType() const override { return std::string( "object" ); }

    void setAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) override
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    constexpr std::string_view childClassKeyword() const override { return DataType::classKeywordStatic(); }

private:
    ChildField( const ChildField& )            = delete;
    ChildField& operator=( const ChildField& ) = delete;

    friend class FieldJsonCap<ChildField<DataType*>>;
    mutable std::unique_ptr<ChildFieldAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildField.inl"
