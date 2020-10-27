#pragma once

#include "cafPdmObjectCapability.h"

#include <QString>

#include <list>
#include <memory>
#include <vector>

class QIODevice;

namespace caf
{
class PdmFieldIoCapability;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmReferenceHelper;
class PdmFieldHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmObjectIoCapability : public PdmObjectCapability
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
    PdmObjectIoCapability( PdmObjectHandle* owner, bool giveOwnership );
    ~PdmObjectIoCapability() override {}

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_XML_HEADER_INIT macro
    virtual QString classKeyword() const                                     = 0;
    virtual bool    matchesClassKeyword( const QString& classKeyword ) const = 0;

    static PdmObjectHandle* readUnknownObjectFromString( const QString&       string,
                                                         PdmObjectFactory*    objectFactory,
                                                         bool                 isCopyOperation,
                                                         IoParameters::IoType ioType = IoParameters::IoType::XML );
    void                    readObjectFromString( const QString&       string,
                                                  PdmObjectFactory*    objectFactory,
                                                  IoParameters::IoType ioType = IoParameters::IoType::XML );
    QString                 writeObjectToString( IoParameters::IoType ioType = IoParameters::IoType::XML ) const;

    PdmObjectHandle* copyBySerialization( PdmObjectFactory*    objectFactory,
                                          IoParameters::IoType ioType = IoParameters::IoType::XML );

    PdmObjectHandle* copyAndCastBySerialization( const QString&       destinationClassKeyword,
                                                 const QString&       sourceClassKeyword,
                                                 PdmObjectFactory*    objectFactory,
                                                 IoParameters::IoType ioType = IoParameters::IoType::XML );

    /// Check if a string is a valid element name
    static bool isValidElementName( const QString& name );

    void initAfterReadRecursively() { initAfterReadRecursively( this->m_owner ); };
    void setupBeforeSaveRecursively() { setupBeforeSaveRecursively( this->m_owner ); };

    void resolveReferencesRecursively( std::vector<PdmFieldHandle*>* fieldWithFailingResolve = nullptr );
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
    void initAfterReadRecursively( PdmObjectHandle* object );
    void setupBeforeSaveRecursively( PdmObjectHandle* object );
    void resolveReferencesRecursively( PdmObjectHandle* object, std::vector<PdmFieldHandle*>* fieldWithFailingResolve );

protected:
    friend class PdmObjectHandle; // Only temporary for void PdmObject::addFieldNoDefault( ) accessing findField

    std::list<QString> m_classInheritanceStack;
    PdmObjectHandle*   m_owner;
};

} // End of namespace caf

#include "cafPdmFieldIoCapability.h"
