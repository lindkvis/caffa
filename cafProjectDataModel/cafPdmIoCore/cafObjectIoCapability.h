#pragma once

#include "cafFieldIoCapability.h"
#include "cafObjectCapability.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace caf
{
class FieldIoCapability;
class ObjectHandle;
class ObjectFactory;
class PdmReferenceHelper;
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

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_IO_HEADER_INIT macro
    virtual std::string classKeyword() const                                         = 0;
    virtual bool        matchesClassKeyword( const std::string& classKeyword ) const = 0;

    static ObjectHandle* readUnknownObjectFromString( const std::string& string,
                                                      ObjectFactory*     objectFactory,
                                                      bool               isCopyOperation,
                                                      IoType             ioType = IoType::JSON );
    void readObjectFromString( const std::string& string, ObjectFactory* objectFactory, IoType ioType = IoType::JSON );
    std::string writeObjectToString( IoType ioType = IoType::JSON, bool writeServerAddress = false ) const;

    ObjectHandle* copyBySerialization( ObjectFactory* objectFactory, IoType ioType = IoType::JSON );

    ObjectHandle* copyAndCastBySerialization( const std::string& destinationClassKeyword,
                                              const std::string& sourceClassKeyword,
                                              ObjectFactory*     objectFactory,
                                              IoType             ioType = IoType::JSON );

    /// Check if a string is a valid element name
    static bool isValidElementName( const std::string& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    void resolveReferencesRecursively( std::vector<FieldHandle*>* fieldWithFailingResolve = nullptr );
    bool inheritsClassWithKeyword( const std::string& testClassKeyword ) const;

    const std::list<std::string>& classInheritanceStack() const;

    bool readFile( const std::string& fileName, IoType ioType = IoType::JSON );
    bool writeFile( const std::string& fileName, IoType ioType = IoType::JSON );

    bool readFile( std::istream& istream, IoType ioType = IoType::JSON);
    bool writeFile( std::ostream& ostream, IoType ioType = IoType::JSON, bool writeServerAddress = false );

protected: // Virtual
    /// Method gets called from PdmDocument after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead(){};
    /// Method gets called from PdmDocument before saving document.
    /// Re-implement to make sure your fields have correct data before saving
    virtual void setupBeforeSave(){};

    bool isInheritedFromSerializable() const { return true; }

    void registerClassKeyword( const std::string& registerKeyword );

private:
    void initAfterReadRecursively( ObjectHandle* object );
    void setupBeforeSaveRecursively( ObjectHandle* object );
    void resolveReferencesRecursively( ObjectHandle* object, std::vector<FieldHandle*>* fieldWithFailingResolve );

protected:
    friend class ObjectHandle; // Only temporary for void Object::addFieldNoDefault( ) accessing findField

    std::list<std::string> m_classInheritanceStack;
    ObjectHandle*          m_owner;
};

} // End of namespace caf
