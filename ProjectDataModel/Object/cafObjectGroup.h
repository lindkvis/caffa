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

    std::vector<Pointer<ObjectHandle>> objects;

    void deleteObjects();
    void addObject( ObjectHandle* obj );

    template <typename T>
    void objectsByType( std::vector<Pointer<T>>* typedObjects ) const
    {
        if ( !typedObjects ) return;
        for ( auto object : objects )
        {
            T* obj = dynamic_cast<T*>( object.p() );
            if ( obj ) typedObjects->push_back( obj );
        }
    }

    template <typename T>
    std::vector<std::unique_ptr<T>> createCopyByType( ObjectFactory* objectFactory ) const;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::unique_ptr<T>> ObjectGroup::createCopyByType( ObjectFactory* objectFactory ) const
{
    std::vector<std::unique_ptr<T>> copyOfTypedObjects;
    std::vector<Pointer<T>>         sourceTypedObjects;
    objectsByType( &sourceTypedObjects );

    for ( Pointer<T> object : sourceTypedObjects )
    {
        if ( object.isNull() ) continue;
        auto        ioCapability = object->template capability<ObjectIoCapability>();
        std::string string       = ioCapability->writeObjectToString();
        auto objectCopy = ObjectIoCapability::readUnknownObjectFromString( string, DefaultObjectFactory::instance(), true );
        auto typedObject = caffa::static_unique_cast<T>( std::move( objectCopy ) );
        CAFFA_ASSERT( typedObject );

        copyOfTypedObjects->push_back( std::move( typedObject ) );
    }
    return copyOfTypedObjects;
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
