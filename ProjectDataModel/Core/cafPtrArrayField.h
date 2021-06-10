#pragma once

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafPointer.h"
#include "cafPortableDataType.h"

#include "cafPtrArrayFieldHandle.h"
namespace caffa
{
template <typename T>
class FieldIoCap;

//==================================================================================================
/// FieldClass to handle a collection of Object derived pointers
/// The reasons for this class is to add itself as parentField into the objects being pointed to.
/// The interface is made similar to std::vector<>, and the complexity of the methods is similar too.
//==================================================================================================

template <typename DataType>
class PtrArrayField : public FieldHandle
{
public:
    PtrArrayField()
    {
        bool doNotUsePtrArrayFieldForAnythingButPointersToObject = false;
        CAFFA_ASSERT( doNotUsePtrArrayFieldForAnythingButPointersToObject );
    }
};

template <typename DataType>
class PtrArrayField<DataType*> : public PtrArrayFieldHandle
{
    typedef DataType* DataTypePtr;

public:
    typedef std::vector<Pointer<DataType>> FieldDataType;

    PtrArrayField()
        : m_isResolved( false )
    {
    }
    virtual ~PtrArrayField();

    PtrArrayField& operator()() { return *this; }

    void                                  setValue( const std::vector<Pointer<DataType>>& fieldValue );
    const std::vector<Pointer<DataType>>& value() const;

    void setValue( const std::vector<DataType*>& fieldValue );

    // Reimplementation of PointersFieldHandle methods

    virtual size_t        size() const { return m_pointers.size(); }
    virtual bool          empty() const { return m_pointers.empty(); }
    virtual void          clear();
    virtual ObjectHandle* at( size_t index );

    // std::vector-like access

    DataType* operator[]( size_t index ) const;

    void   push_back( DataType* pointer );
    void   set( size_t index, DataType* pointer );
    void   insert( size_t indexAfter, DataType* pointer );
    void   insert( size_t indexAfter, const std::vector<Pointer<DataType>>& objects );
    size_t count( const DataType* pointer ) const;

    void   erase( size_t index );
    size_t index( DataType* pointer );
    void   removePtr( ObjectHandle* object );

    typename std::vector<Pointer<DataType>>::iterator begin() { return m_pointers.begin(); };
    typename std::vector<Pointer<DataType>>::iterator end() { return m_pointers.end(); };

    typename std::vector<Pointer<DataType>>::const_iterator begin() const { return m_pointers.begin(); };
    typename std::vector<Pointer<DataType>>::const_iterator end() const { return m_pointers.end(); };

    std::vector<DataType*> ptrReferencedObjects() const;

    // Child objects
    virtual void ptrReferencedObjects( std::vector<ObjectHandle*>* );

    std::string dataType() const override { return std::string( "object[]:" ) + DataType::classKeywordStatic(); }

private: // To be disabled
    CAFFA_DISABLE_COPY_AND_ASSIGN( PtrArrayField );

    void addThisAsReferencingPtrField();
    void removeThisAsReferencingPtrField();

private:
    friend class FieldIoCap<PtrArrayField<DataType*>>;

    std::vector<Pointer<DataType>> m_pointers;
    std::string                    m_referenceString;
    bool                           m_isResolved;
};

} // End of namespace caffa

#include "cafPtrArrayField.inl"
