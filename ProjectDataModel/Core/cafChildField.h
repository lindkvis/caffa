#pragma once

#include "cafFieldHandle.h"

#include "cafAssert.h"
#include "cafChildFieldAccessor.h"
#include "cafPointer.h"
#include "cafPortableDataType.h"

#include <memory>

namespace caffa
{
template <typename T>
class FieldIoCap;
//==================================================================================================
/// Specialization for pointers, but only applicable to Object derived objects.
/// The pointer is guarded, meaning that it will be set to nullptr if the object pointed to
/// is deleted. The referenced object will be printed in place in the xml-file
/// This is supposed to be renamed to ChildField
//==================================================================================================

class ChildFieldHandle : public FieldHandle
{
public:
    virtual void setChildObject( std::unique_ptr<ObjectHandle> object )               = 0;
    virtual void setFieldDataAccessor( std::unique_ptr<ChildFieldAccessor> accessor ) = 0;
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

    explicit ChildField( DataTypePtr fieldValue );
    virtual ~ChildField();

    // Assignment

    ChildField& operator=( DataTypePtr fieldValue );

    // Basic access

    DataType* value() const { return static_cast<DataType*>( m_fieldDataAccessor->value() ); }
    void      setValue( DataTypePtr fieldValue );

    // Access operators

    /*Conversion*/ operator DataType*() const { return this->value(); }
    DataType*      operator->() const { return this->value(); }

    const DataType* operator()() const { return static_cast<const DataType*>( m_fieldDataAccessor->value() ); }

    // Child objects
    std::vector<ObjectHandle*>    childObjects() const override;
    void                          childObjects( std::vector<ObjectHandle*>* objects ) const override;
    std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;
    void                          setChildObject( std::unique_ptr<ObjectHandle> object );

    std::string dataType() const override { return std::string( "object" ); }

    void setFieldDataAccessor( std::unique_ptr<ChildFieldAccessor> accessor )
    {
        m_fieldDataAccessor = std::move( accessor );
    }

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( ChildField );

    friend class FieldIoCap<ChildField<DataType*>>;
    std::unique_ptr<ChildFieldAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildField.inl"
