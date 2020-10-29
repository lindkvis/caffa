#include <thread>

#include "cafAsyncWorkerManager.h"
#include "cafObjectHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Constructor that takes ownership of the data in the provided vector
//--------------------------------------------------------------------------------------------------
template <typename ObjectType>
AsyncObjectVectorDeleter<ObjectType>::AsyncObjectVectorDeleter( std::vector<ObjectType*>& pointerVector )
{
    m_pointersToDelete.reserve( pointerVector.size() );
    for ( ObjectType* rawPointer : pointerVector )
    {
        if ( rawPointer )
        {
            ObjectHandle* objectHandle = static_cast<ObjectHandle*>( rawPointer );
            objectHandle->prepareForDelete();
            m_pointersToDelete.push_back( objectHandle );
        }
    }
    pointerVector.clear();
}

//--------------------------------------------------------------------------------------------------
/// Constructor that takes ownership of the data in the provided vector
//--------------------------------------------------------------------------------------------------
template <typename ObjectType>
AsyncObjectVectorDeleter<ObjectType>::AsyncObjectVectorDeleter( std::vector<PdmPointer<ObjectType>>& pdmPointerVector )
{
    m_pointersToDelete.reserve( pdmPointerVector.size() );
    for ( PdmPointer<ObjectType>& pdmPointer : pdmPointerVector )
    {
        if ( pdmPointer.notNull() )
        {
            ObjectHandle* objectHandle = pdmPointer.rawPtr();
            objectHandle->prepareForDelete();
            m_pointersToDelete.push_back( objectHandle );
        }
    }
    pdmPointerVector.clear();
}

//--------------------------------------------------------------------------------------------------
/// Destructor will launch the asynchronous deletion if start() hasn't already been run.
//--------------------------------------------------------------------------------------------------
template <typename ObjectType>
AsyncObjectVectorDeleter<ObjectType>::~AsyncObjectVectorDeleter()
{
    if ( !m_pointersToDelete.empty() )
    {
        start();
    }
}

//--------------------------------------------------------------------------------------------------
/// Perform deletion of the pointers in a separate thread.
//--------------------------------------------------------------------------------------------------
template <typename ObjectType>
void AsyncObjectVectorDeleter<ObjectType>::start()
{
    std::thread thread(
        []( std::vector<ObjectHandle*>&& pointerVector ) {
            for ( ObjectHandle* pointerToDelete : pointerVector )
            {
                delete pointerToDelete;
            }
        },
        std::move( m_pointersToDelete ) );
    AsyncWorkerManager::instance().takeThreadOwnership( thread, false );
}

} // namespace caf