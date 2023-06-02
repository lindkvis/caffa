#pragma once

#include "cafObjectCapability.h"
#include "cafObjectHandle.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace caffa
{
class ObjectHandle;
class ObjectFactory;
class Serializer;

//==================================================================================================
//
//
//
//==================================================================================================
class ObjectIoCapability : public ObjectCapability
{
public:
    ObjectIoCapability( ObjectHandle* owner );
    ~ObjectIoCapability() noexcept override;

    void initAfterReadRecursively();
    void setupBeforeSaveRecursively();

    ObjectHandle::Ptr copyBySerialization( ObjectFactory* objectFactory ) const;

    ObjectHandle::Ptr copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                  ObjectFactory*     objectFactory ) const;

    /**
     * @brief Copy the object by serializing it and then casting it to the given type.
     * Note that the object will be deleted if the cast fails.
     *
     * @tparam ObjectType The type (derived from ObjectHandle) to cast to
     * @param objectFactory The factory used to create the object
     * @return std::shared_ptr<ObjectType>
     */
    template <typename ObjectType>
    std::shared_ptr<ObjectType> copyTypedObjectBySerialization( ObjectFactory* objectFactory ) const
    {
        auto objectHandle = this->copyBySerialization( objectFactory );
        return std::dynamic_pointer_cast<ObjectType>( objectHandle );
    }

    bool readFile( const std::string& fileName );
    bool writeFile( const std::string& fileName );

    bool readStream( std::istream& inStream );
    bool writeStream( std::ostream& outStream );

    bool readStream( std::istream& inStream, const Serializer& serializer );
    bool writeStream( std::ostream& outStream, const Serializer& serializer );

private:
    static void initAfterReadRecursively( ObjectHandle* object );
    static void setupBeforeSaveRecursively( ObjectHandle* object );

protected:
    ObjectHandle* m_owner;
};

} // End of namespace caffa
