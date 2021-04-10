#pragma once

#include "cafFieldHandle.h"

#include "cafAssert.h"
#include "cafPointer.h"

namespace caf
{
template <typename T>
class FieldIoCap;
//==================================================================================================
/// Specialization for pointers, but only applicable to Object derived objects.
/// The pointer is guarded, meaning that it will be set to NULL if the object pointed to
/// is deleted. The referenced object will be printed in place in the xml-file
/// This is supposed to be renamed to ChildField
//==================================================================================================

class ChildFieldHandle : public FieldHandle
{
public:
    virtual void childObjects( std::vector<ObjectHandle*>* objects ) = 0;
    virtual void setChildObject( ObjectHandle* object )              = 0;
};

template <typename DataType>
class ChildField : public ChildFieldHandle
{
public:
    using FieldDataType = DataType*;

    ChildField()
    {
        bool doNotUsePtrFieldForAnythingButPointersToObject = false;
        CAF_ASSERT( doNotUsePtrFieldForAnythingButPointersToObject );
    }
};

template <typename DataType>
class ChildField<DataType*> : public ChildFieldHandle
{
    typedef DataType* DataTypePtr;

public:
    using FieldDataType = DataType*;

    ChildField() {}
    explicit ChildField( const DataTypePtr& fieldValue );
    virtual ~ChildField();

    // Assignment

    ChildField& operator=( const DataTypePtr& fieldValue );

    // Basic access

    DataType* value() const { return m_fieldValue; }
    void      setValue( const DataTypePtr& fieldValue );

    // Access operators

    /*Conversion*/ operator DataType*() const { return m_fieldValue; }
    DataType*      operator->() const { return m_fieldValue; }

    const Pointer<DataType>& operator()() const { return m_fieldValue; }
    const Pointer<DataType>& v() const { return m_fieldValue; }

    // Child objects
    virtual void childObjects( std::vector<ObjectHandle*>* objects ) override;
    void         setChildObject( ObjectHandle* object ) override;
    virtual void removeChildObject( ObjectHandle* object ) override;

private:
    PDM_DISABLE_COPY_AND_ASSIGN( ChildField );

    friend class FieldIoCap<ChildField<DataType*>>;
    Pointer<DataType> m_fieldValue;
};

} // End of namespace caf

#include "cafChildField.inl"
