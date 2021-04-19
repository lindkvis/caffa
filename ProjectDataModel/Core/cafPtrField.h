#pragma once

#include "cafAssert.h"
#include "cafPointer.h"
#include "cafValueField.h"

#include <any>

namespace caffa
{
template <typename T>
class FieldIoCap;

//==================================================================================================
/// A field that contains a pointer to a ObjectHandle derived object.
/// The referenced object will not be printed in the XML-output yet, but
/// it is intended to be written as a reference (by path from common root)
/// This field has nothing to do with ownership at all, and is not a part of the
/// parent-child relations induced by the other ChildField<PtrType*> ChildArrayField<PtrType*>
/// The pointer is guarded, meaning that it will be set to nullptr if the object pointed to
/// is deleted.
//==================================================================================================

template <typename DataType>
class PtrField : public ValueField
{
public:
    PtrField()
    {
        bool doNotUsePtrFieldForAnythingButPointersToObject = false;
        CAFFA_ASSERT( doNotUsePtrFieldForAnythingButPointersToObject );
    }
};

template <typename DataType>
class PtrField<DataType*> : public ValueField
{
    typedef DataType* DataTypePtr;

public:
    typedef Pointer<DataType> FieldDataType;

    PtrField()
        : m_isResolved( false )
    {
    }
    explicit PtrField( const DataTypePtr& fieldValue );
    ~PtrField() override;

    //  Assignment

    PtrField& operator=( const DataTypePtr& fieldValue );
    PtrField& operator=( const FieldDataType& fieldValue );

    // Basic access

    DataType* value() const { return m_fieldValue; }
    void      setValue( const DataTypePtr& fieldValue );

    // Variant access
    Variant toVariant() const override;
    void    setFromVariant( const Variant& variant ) override;
    bool    isReadOnly() const override { return false; }

    // Access operators

    /*Conversion*/ operator DataType*() const { return m_fieldValue; }
    DataType*      operator->() const { return m_fieldValue; }
    DataType*      operator()() const { return m_fieldValue; }

    bool operator==( const DataTypePtr& fieldValue ) { return m_fieldValue == fieldValue; }

    // Ptr referenced objects

    void ptrReferencedObjects( std::vector<ObjectHandle*>* objectsToFill ) override;

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( PtrField );

    friend class FieldIoCap<PtrField<DataType*>>;
    void setRawPtr( ObjectHandle* obj );

    Pointer<DataType> m_fieldValue;

    // Resolving
    std::string m_referenceString;
    bool        m_isResolved;
};

} // End of namespace caffa

#include "cafPtrField.inl"
