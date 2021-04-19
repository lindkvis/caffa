#pragma once

#include "cafChildArrayField.h"
#include "cafObject.h"

namespace caffa
{
class ReferenceHelper;

//==================================================================================================
/// The ObjectGroup serves as a container of unknown Objects
//==================================================================================================
class ObjectGroup : public Object
{
    CAFFA_HEADER_INIT;

public:
    ObjectGroup();
    ~ObjectGroup() override;

    std::vector<ObjectHandle*> objects;

    void deleteObjects();
    void addObject( ObjectHandle* obj );

    template <typename T>
    void objectsByType( std::vector<Pointer<T>>* typedObjects ) const
    {
        if ( !typedObjects ) return;
        size_t it;
        for ( it = 0; it != objects.size(); ++it )
        {
            T* obj = dynamic_cast<T*>( objects[it] );
            if ( obj ) typedObjects->push_back( obj );
        }
    }

    template <typename T>
    void createCopyByType( std::vector<Pointer<T>>* copyOfTypedObjects, ObjectFactory* objectFactory ) const;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void ObjectGroup::createCopyByType( std::vector<Pointer<T>>* copyOfTypedObjects, ObjectFactory* objectFactory ) const
{
    std::vector<Pointer<T>> sourceTypedObjects;
    objectsByType( &sourceTypedObjects );

    for ( size_t i = 0; i < sourceTypedObjects.size(); i++ )
    {
        auto          ioCapability = sourceTypedObjects[i]->template capability<ObjectIoCapability>();
        std::string   string       = ioCapability->writeObjectToString();
        ObjectHandle* objectCopy =
            ObjectIoCapability::readUnknownObjectFromString( string, DefaultObjectFactory::instance(), true );

        T* typedObject = dynamic_cast<T*>( objectCopy );
        CAFFA_ASSERT( typedObject );

        copyOfTypedObjects->push_back( typedObject );
    }
}

//==================================================================================================
/// The ObjectCollection serves as a container of unknown Objects stored in a ChildArrayField
//==================================================================================================
class ObjectCollection : public Object
{
    CAFFA_HEADER_INIT;

public:
    ObjectCollection();
    ~ObjectCollection() override;

    caffa::ChildArrayField<ObjectHandle*> objects;
};

} // End of namespace caffa
