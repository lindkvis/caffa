#pragma once

#include "cafObjectCapability.h"

#include <QString>

#include <list>
#include <memory>
#include <vector>

class QIODevice;

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
    struct IoParameters
    {
    public:
        enum class IoType
        {
            XML,
            JSON,
            SQL // Not yet implemented
        };

        IoType     ioType;
        QIODevice* ioDevice;
    };

public:
    ObjectIoCapability( ObjectHandle* owner, bool giveOwnership );
    ~ObjectIoCapability() override {}

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_XML_HEADER_INIT macro
    virtual QString classKeyword() const                                     = 0;
    virtual bool    matchesClassKeyword( const QString& classKeyword ) const = 0;

    static ObjectHandle* readUnknownObjectFromString( const QString&       string,
                                                         ObjectFactory*    objectFactory,
                                                         bool                 isCopyOperation,
                                                         IoParameters::IoType ioType = IoParameters::IoType::XML );
    void                    readObjectFromString( const QString&       string,
                                                  ObjectFactory*    objectFactory,
                                                  IoParameters::IoType ioType = IoParameters::IoType::XML );
    QString                 writeObjectToString( IoParameters::IoType ioType = IoParameters::IoType::XML ) const;

    ObjectHandle* copyBySerialization( ObjectFactory*    objectFactory,
                                          IoParameters::IoType ioType = IoParameters::IoType::XML );

    ObjectHandle* copyAndCastBySerialization( const QString&       destinationClassKeyword,
                                                 const QString&       sourceClassKeyword,
                                                 ObjectFactory*    objectFactory,
                                                 IoParameters::IoType ioType = IoParameters::IoType::XML );

    /// Check if a string is a valid element name
    static bool isValidElementName( const QString& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    void resolveReferencesRecursively( std::vector<FieldHandle*>* fieldWithFailingResolve = nullptr );
    bool inheritsClassWithKeyword( const QString& testClassKeyword ) const;

    const std::list<QString>& classInheritanceStack() const;

    virtual void readFile( const IoParameters& parameters );
    virtual void writeFile( const IoParameters& parameters );

protected: // Virtual
    /// Method gets called from PdmDocument after all objects are read.
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void initAfterRead(){};
    /// Method gets called from PdmDocument before saving document.
    /// Re-implement to make sure your fields have correct data before saving
    virtual void setupBeforeSave(){};

    bool isInheritedFromSerializable() const { return true; }

    void registerClassKeyword( const QString& registerKeyword );

private:
    void initAfterReadRecursively( ObjectHandle* object );
    void setupBeforeSaveRecursively( ObjectHandle* object );
    void resolveReferencesRecursively( ObjectHandle* object, std::vector<FieldHandle*>* fieldWithFailingResolve );

protected:
    friend class ObjectHandle; // Only temporary for void Object::addFieldNoDefault( ) accessing findField

    std::list<QString> m_classInheritanceStack;
    ObjectHandle*   m_owner;
};

} // End of namespace caf

#include "cafFieldIoCapability.h"
