#pragma once

#include "cafDynamicUniqueCast.h"
#include "cafObjectCapability.h"

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
    enum class IoType
    {
        JSON
    };

public:
    ObjectIoCapability( ObjectHandle* owner );
    ~ObjectIoCapability() noexcept override;

    /// Check if a string is a valid element name
    static bool isValidElementName( const std::string& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    std::unique_ptr<ObjectHandle> copyBySerialization( ObjectFactory* objectFactory ) const;

    std::unique_ptr<ObjectHandle> copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                              ObjectFactory*     objectFactory ) const;

    /**
     * @brief Copy the object by serializing it and then casting it to the given type.
     * Note that the object will be deleted if the cast fails.
     *
     * @tparam ObjectType The type (derived from ObjectHandle) to cast to
     * @param objectFactory The factory used to create the object
     * @return std::unique_ptr<ObjectType>
     */
    template <typename ObjectType>
    std::unique_ptr<ObjectType> copyTypedObjectBySerialization( ObjectFactory* objectFactory ) const
    {
        auto objectHandle = this->copyBySerialization( objectFactory );
        return caffa::dynamic_unique_cast<ObjectType>( std::move( objectHandle ) );
    }

    bool readFile( const std::string& fileName, IoType ioType = IoType::JSON );
    bool writeFile( const std::string& fileName, IoType ioType = IoType::JSON );

    bool readStream( std::istream& inStream, IoType ioType = IoType::JSON );
    bool writeStream( std::ostream& outStream, IoType ioType = IoType::JSON );

    bool readStream( std::istream& inStream, const Serializer& serializer );
    bool writeStream( std::ostream& outStream, const Serializer& serializer );

protected: // Virtual
    /// Method gets called from Document after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead(){};
    /// Method gets called from Document before saving document.
    /// Re-implement to make sure your fields have correct data before saving
    virtual void setupBeforeSave(){};

    bool isInheritedFromSerializable() const { return true; }

private:
    void initAfterReadRecursively( ObjectHandle* object );
    void setupBeforeSaveRecursively( ObjectHandle* object );

protected:
    ObjectHandle* m_owner;
};

} // End of namespace caffa
