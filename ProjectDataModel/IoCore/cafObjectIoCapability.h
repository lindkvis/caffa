#pragma once

#include "cafFieldIoCapability.h"
#include "cafObjectCapability.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace caffa
{
class FieldIoCapability;
class ObjectHandle;
class ObjectFactory;
class ReferenceHelper;
class FieldHandle;

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
        JSON,
        SQL // Not yet implemented
    };

public:
    ObjectIoCapability( ObjectHandle* owner, bool giveOwnership );
    ~ObjectIoCapability() override {}

    /// The classKeyword method is overridden in subclasses by the CAFFA_IO_HEADER_INIT macro
    virtual std::string              classKeyword() const                                         = 0;
    virtual bool                     matchesClassKeyword( const std::string& classKeyword ) const = 0;
    virtual std::vector<std::string> classInheritanceStack() const                                = 0;

    static std::unique_ptr<ObjectHandle> readUnknownObjectFromString( const std::string& string,
                                                                      ObjectFactory*     objectFactory,
                                                                      bool               copyDataValues,
                                                                      IoType             ioType = IoType::JSON );
    void readObjectFromString( const std::string& string, ObjectFactory* objectFactory, IoType ioType = IoType::JSON );
    std::string writeObjectToString( IoType ioType = IoType::JSON ) const;

    std::unique_ptr<ObjectHandle> copyBySerialization( ObjectFactory* objectFactory, IoType ioType = IoType::JSON );

    std::unique_ptr<ObjectHandle> copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                                              const std::string& sourceClassKeyword,
                                                              ObjectFactory*     objectFactory,
                                                              IoType             ioType = IoType::JSON );

    template <typename ObjectType>
    std::unique_ptr<ObjectType> copyTypedObjectBySerialization( ObjectFactory* objectFactory, IoType ioType = IoType::JSON )
    {
        auto objectHandle = this->copyBySerialization( objectFactory, ioType );
        if ( dynamic_cast<ObjectType*>( objectHandle.get() ) != nullptr )
        {
            return std::unique_ptr<ObjectType>( static_cast<ObjectType*>( objectHandle.release() ) );
        }
        return nullptr; // Will delete the copy
    }

    /// Check if a string is a valid element name
    static bool isValidElementName( const std::string& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    bool readFile( const std::string& fileName, IoType ioType = IoType::JSON );
    bool writeFile( const std::string& fileName, IoType ioType = IoType::JSON );

    bool readFile( std::istream& istream, IoType ioType = IoType::JSON );
    bool writeFile( std::ostream& ostream, IoType ioType = IoType::JSON );

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
