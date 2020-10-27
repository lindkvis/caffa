#pragma once

#include "cafPdmObjectCapability.h"
#include "cafPdmObjectIoCapability.h"

#include <QJsonObject>
#include <QString>

#include <list>
#include <vector>

class QIODevice;
class QJsonStreamReader;
class QJsonStreamWriter;

namespace caf
{
class PdmFieldJsonCapability;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmReferenceHelper;
class PdmFieldHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmObjectJsonCapability
{
public:
    PdmObjectJsonCapability( PdmObjectHandle* owner, bool giveOwnership );
    ~PdmObjectJsonCapability() override {}

    /// Convenience methods to serialize/de-serialize this particular object (with children)
    void        readObjectFromJson( const QJsonObject& jsonObject, PdmObjectFactory* objectFactory );
    QJsonObject writeObjectToJson() const;

    PdmObjectHandle* copyBySerialization( PdmObjectFactory* objectFactory ) override;
    PdmObjectHandle* copyAndCastBySerialization( const QString&    destinationClassKeyword,
                                                 const QString&    sourceClassKeyword,
                                                 PdmObjectFactory* objectFactory ) override;

protected:
    /// This method is intended to be used in macros to make compile time errors
    // if user uses them on wrong type of objects
    bool isInheritedFromSerializable() { return true; }

private:
    // Main Json serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    void readFields( QJsonStreamReader& inputStream, PdmObjectFactory* objectFactory, bool isCopyOperation );
    void writeFields( QJsonStreamWriter& outputStream ) const;

    friend class PdmObjectHandle; // Only temporary for void PdmObject::addFieldNoDefault( ) accessing findField
};

PdmObjectJsonCapability* JsonObj( PdmObjectHandle* obj );

} // End of namespace caf

#include "cafPdmFieldJsonCapability.h"
