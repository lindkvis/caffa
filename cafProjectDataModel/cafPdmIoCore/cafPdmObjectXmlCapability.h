#pragma once

#include "cafPdmObjectCapability.h"
#include "cafPdmObjectIoCapability.h"

#include <QString>

#include <list>
#include <vector>

class QIODevice;
class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{
class PdmFieldXmlCapability;
class PdmObjectIoCapability;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmReferenceHelper;
class PdmFieldHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmObjectXmlCapability
{
public:
    /// Convenience methods to serialize/de-serialize this particular object (with children)
    static void readObjectFromString( PdmObjectHandle* object, const QString& xmlString, PdmObjectFactory* objectFactory );
    static QString          writeObjectToString( const PdmObjectHandle* object );
    static PdmObjectHandle* copyByXmlSerialization( const PdmObjectHandle* object, PdmObjectFactory* objectFactory );
    static PdmObjectHandle* copyAndCastByXmlSerialization( const PdmObjectHandle* object,
                                                           const QString&         destinationClassKeyword,
                                                           const QString&         sourceClassKeyword,
                                                           PdmObjectFactory*      objectFactory );

    static PdmObjectHandle*
        readUnknownObjectFromXmlString( const QString& xmlString, PdmObjectFactory* objectFactory, bool isCopyOperation );

    static void readFile( PdmObjectHandle* object, QIODevice* file );
    static void writeFile( const PdmObjectHandle* object, QIODevice* file );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    static void readFields( PdmObjectHandle*  object,
                            QXmlStreamReader& inputStream,
                            PdmObjectFactory* objectFactory,
                            bool              isCopyOperation );
    static void writeFields( const PdmObjectHandle* object, QXmlStreamWriter& outputStream );
};

} // End of namespace caf
