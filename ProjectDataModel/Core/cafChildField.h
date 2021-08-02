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
    virtual void setChildObject( std::unique_ptr<ObjectHandle> object ) = 0;
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
    using FieldDataType         = DataType*;
    using DataAccessor          = ChildFieldAccessor<DataType>;
    using DirectStorageAccessor = ChildFieldDirectStorageAccessor<DataType>;

    ChildField()
        : m_fieldDataAccessor( std::make_unique<DirectStorageAccessor>( this ) )
    {
    }
    ChildField( std::unique_ptr<DataAccessor> accessor )
        : m_fieldDataAccessor( std::move( accessor ) )
    {
    }

    explicit ChildField( DataTypePtr fieldValue );
    virtual ~ChildField();

    // Assignment

    ChildField& operator=( DataTypePtr fieldValue );

    // Basic access

    DataType* value() const { return m_fieldDataAccessor->value(); }
    void      setValue( DataTypePtr fieldValue );

    // Access operators

    /*Conversion*/ operator DataType*() const { return m_fieldDataAccessor->value(); }
    DataType*      operator->() const { return m_fieldDataAccessor->value(); }

    const DataType* operator()() const { return m_fieldDataAccessor->value(); }

    // Child objects
    std::vector<ObjectHandle*>    childObjects() const override;
    void                          childObjects( std::vector<ObjectHandle*>* objects ) const override;
    std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;
    void                          setChildObject( std::unique_ptr<ObjectHandle> object );

    std::string dataType() const override { return std::string( "object" ); }

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( ChildField );

    friend class FieldIoCap<ChildField<DataType*>>;
    std::unique_ptr<DataAccessor> m_fieldDataAccessor;
};

} // End of namespace caffa

#include "cafChildField.inl"
