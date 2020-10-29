#include <QVariant>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
QVariant caf::PtrField<DataType*>::toQVariant() const
{
    caf::ObjectHandle*                 objectHandle = m_fieldValue.rawPtr();
    caf::PdmPointer<caf::ObjectHandle> ptrHandle( objectHandle );
    return QVariant::fromValue( ptrHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PtrField<DataType*>::setFromQVariant( const QVariant& variant )
{
    caf::PdmPointer<caf::ObjectHandle> variantHandle = variant.value<caf::PdmPointer<caf::ObjectHandle>>();
    m_fieldValue.setRawPtr( variantHandle.rawPtr() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PtrField<DataType*>::PtrField( const DataTypePtr& fieldValue )
{
    m_isResolved = true;
    m_fieldValue = fieldValue;
    if ( m_fieldValue != NULL ) m_fieldValue->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PtrField<DataType*>::~PtrField()
{
    if ( !m_fieldValue.isNull() ) m_fieldValue.rawPtr()->removeReferencingPtrField( this );
    m_fieldValue.setRawPtr( NULL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrField<DataType*>::setValue( const DataTypePtr& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != NULL ) m_fieldValue->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrField<DataType*>::setRawPtr( ObjectHandle* obj )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue.notNull() ) m_fieldValue.rawPtr()->removeReferencingPtrField( this );
    m_fieldValue.setRawPtr( obj );
    if ( m_fieldValue.notNull() ) m_fieldValue.rawPtr()->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PtrField<DataType*>& PtrField<DataType*>::operator=( const DataTypePtr& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != NULL ) m_fieldValue->addReferencingPtrField( this );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PtrField<DataType*>& PtrField<DataType*>::operator=( const FieldDataType& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != NULL ) m_fieldValue->addReferencingPtrField( this );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrField<DataType*>::ptrReferencedObjects( std::vector<ObjectHandle*>* objectsToFill )
{
    if ( m_fieldValue.rawPtr() )
    {
        objectsToFill->push_back( m_fieldValue.rawPtr() );
    }
}

} // End of namespace caf
