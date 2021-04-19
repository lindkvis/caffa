namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
Variant caffa::PtrField<DataType*>::toVariant() const
{
    caffa::ObjectHandle*              objectHandle = m_fieldValue.rawPtr();
    caffa::Pointer<caffa::ObjectHandle> ptrHandle( objectHandle );
    return Variant( ptrHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caffa::PtrField<DataType*>::setFromVariant( const Variant& variant )
{
    try
    {
        caffa::Pointer<caffa::ObjectHandle> variantHandle = variant.value<caffa::Pointer<caffa::ObjectHandle>>();
        m_fieldValue.setRawPtr( variantHandle.rawPtr() );
    }
    catch ( std::bad_any_cast& )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caffa::PtrField<DataType*>::PtrField( const DataTypePtr& fieldValue )
{
    m_isResolved = true;
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caffa::PtrField<DataType*>::~PtrField()
{
    if ( !m_fieldValue.isNull() ) m_fieldValue.rawPtr()->removeReferencingPtrField( this );
    m_fieldValue.setRawPtr( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrField<DataType*>::setValue( const DataTypePtr& fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PtrField<DataType*>::setRawPtr( ObjectHandle* obj )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue.notNull() ) m_fieldValue.rawPtr()->removeReferencingPtrField( this );
    m_fieldValue.setRawPtr( obj );
    if ( m_fieldValue.notNull() ) m_fieldValue.rawPtr()->addReferencingPtrField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caffa::PtrField<DataType*>& PtrField<DataType*>::operator=( const DataTypePtr& fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->addReferencingPtrField( this );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caffa::PtrField<DataType*>& PtrField<DataType*>::operator=( const FieldDataType& fieldValue )
{
    CAFFA_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeReferencingPtrField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->addReferencingPtrField( this );

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

} // End of namespace caffa
