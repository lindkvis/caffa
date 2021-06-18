#pragma once

#include "cafFieldHandle.h"

#include "cafAssert.h"
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
    virtual void childObjects( std::vector<ObjectHandle*>* objects ) = 0;
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

    ChildField() {}
    explicit ChildField( DataTypePtr fieldValue );
    virtual ~ChildField();

    // Assignment

    ChildField& operator=( DataTypePtr fieldValue );

    // Basic access

    DataType* value() const { return m_fieldValue; }
    DataType* setValue( DataTypePtr fieldValue );

    // Access operators

    /*Conversion*/ operator DataType*() const { return m_fieldValue; }
    DataType*      operator->() const { return m_fieldValue; }

    const Pointer<DataType>& operator()() const { return m_fieldValue; }

    // Child objects
    virtual void                                childObjects( std::vector<ObjectHandle*>* objects ) override;
    [[nodiscard]] std::unique_ptr<DataType>     remove( ObjectHandle* object );
    [[nodiscard]] std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* object ) override;

    std::string dataType() const override { return std::string( "object" ); }

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( ChildField );

    friend class FieldIoCap<ChildField<DataType*>>;
    Pointer<DataType> m_fieldValue;
};

} // End of namespace caffa

#include "cafChildField.inl"
