#pragma once

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
    ObjectIoCapability( ObjectHandle* owner, bool giveOwnership );
    ~ObjectIoCapability() noexcept override;

    /// The classKeyword method is overridden in subclasses by the CAFFA_HEADER_INIT macro
    virtual std::string              classKeyword() const                                         = 0;
    virtual bool                     matchesClassKeyword( const std::string& classKeyword ) const = 0;
    virtual std::vector<std::string> classInheritanceStack() const                                = 0;

    /// Check if a string is a valid element name
    static bool isValidElementName( const std::string& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    std::unique_ptr<ObjectHandle> copyBySerialization( ObjectFactory* objectFactory );

    std::unique_ptr<ObjectHandle> copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                              ObjectFactory*     objectFactory );

    template <typename ObjectType>
    std::unique_ptr<ObjectType> copyTypedObjectBySerialization( ObjectFactory* objectFactory )
    {
        auto objectHandle = this->copyBySerialization( objectFactory );
        if ( dynamic_cast<ObjectType*>( objectHandle.get() ) != nullptr )
        {
            return std::unique_ptr<ObjectType>( static_cast<ObjectType*>( objectHandle.release() ) );
        }
        return nullptr; // Will delete the copy
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
    friend class ObjectHandle; // Only temporary for void Object::addFieldNoDefault( ) accessing findField

    ObjectHandle* m_owner;
};

} // End of namespace caffa
