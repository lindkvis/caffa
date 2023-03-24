#pragma once

#include "cafAssert.h"
#include "cafChildFieldAccessor.h"
#include "cafFieldHandle.h"
#include "cafObservingPointer.h"
#include "cafPortableDataType.h"

#include <memory>

namespace caffa
{
template <typename T>
class FieldJsonCap;
//==================================================================================================
/// Specialization for pointers, but only applicable to Object derived objects.
/// The pointer is guarded, meaning that it will be set to nullptr if the object pointed to
/// is deleted. The referenced object will be printed in place in the xml-file
/// This is supposed to be renamed to ChildField
//==================================================================================================

class ChildFieldHandle : public FieldHandle
{
public:
    virtual void                          setChildObject( std::unique_ptr<ObjectHandle> object )      = 0;
    virtual void                          setAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) = 0;
    virtual std::unique_ptr<ObjectHandle> clear()                                                     = 0;
    virtual constexpr std::string_view    childClassKeyword() const                                   = 0;
};

template <typename DataType>
class ChildField : public ChildFieldHandle
{
public:
    using FieldDataType = DataType*;

    ChildField()
    {
        bool doNotUseChildFieldForAnythingButPointersToObject = false;
        CAFFA_ASSERT( doNotUseChildFieldForAnythingButPointersToObject );
    }
};

template <typename DataType>
class ChildField<DataType*> : public ChildFieldHandle
{
    using DataTypePtr = std::unique_ptr<DataType>;

public:
    using FieldDataType = DataType*;

    ChildField()
        : m_fieldDataAccessor( std::make_unique<ChildFieldDirectStorageAccessor>( this ) )
    {
    }

    virtual ~ChildField();

    // Assignment

    ChildField& operator=( DataTypePtr object );

    // Basic access

    DataType*       object() { return static_cast<DataType*>( m_fieldDataAccessor->object() ); }
    const DataType* object() const { return static_cast<const DataType*>( m_fieldDataAccessor->object() ); }
    void            setObject( DataTypePtr object );

    // Access operators
    operator DataType*() { return this->object(); }
    operator const DataType*() const { return this->object(); }

    // Deep copy of object content
    std::unique_ptr<DataType> deepCloneObject() const;
    void                      deepCopyObjectFrom( const DataType* copyFrom );

    DataType*       operator->() { return this->object(); }
    const DataType* operator->() const { return this->object(); }

    DataType*       operator()() { return static_cast<DataType*>( m_fieldDataAccessor->object() ); }
    const DataType* operator()() const { return static_cast<const DataType*>( m_fieldDataAccessor->object() ); }

    // Child objects
    std::vector<ObjectHandle*>       childObjects() override;
    std::vector<const ObjectHandle*> childObjects() const override;
    std::unique_ptr<ObjectHandle>    clear() override;
    std::unique_ptr<ObjectHandle>    removeChildObject( ObjectHandle* object ) override;
    void                             setChildObject( std::unique_ptr<ObjectHandle> object ) override;

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
