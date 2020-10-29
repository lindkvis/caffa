#pragma once

#include "cafObjectCapability.h"
#include "cafObjectIoCapability.h"

#include <QString>

#include <list>
#include <vector>

class QIODevice;
class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{
class FieldXmlCapability;
class ObjectIoCapability;
class ObjectHandle;
class ObjectFactory;
class PdmReferenceHelper;
class FieldHandle;

//==================================================================================================
//
//
//
//==================================================================================================
class ObjectXmlCapability
{
public:
    /// Convenience methods to serialize/de-serialize this particular object (with children)
    static void
        readObjectFromXmlString( ObjectHandle* object, const QString& xmlString, ObjectFactory* objectFactory );
    static QString          writeObjectToXmlString( const ObjectHandle* object );
    static ObjectHandle* copyByXmlSerialization( const ObjectHandle* object, ObjectFactory* objectFactory );
    static ObjectHandle* copyAndCastByXmlSerialization( const ObjectHandle* object,
                                                           const QString&         destinationClassKeyword,
                                                           const QString&         sourceClassKeyword,
                                                           ObjectFactory*      objectFactory );

    static ObjectHandle*
        readUnknownObjectFromXmlString( const QString& xmlString, ObjectFactory* objectFactory, bool isCopyOperation );

    static void readFile( ObjectHandle* object, QIODevice* file );
    static void writeFile( const ObjectHandle* object, QIODevice* file );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    static void readFields( ObjectHandle*  object,
                            QXmlStreamReader& inputStream,
                            ObjectFactory* objectFactory,
                            bool              isCopyOperation );
    static void writeFields( const ObjectHandle* object, QXmlStreamWriter& outputStream );
};

} // End of namespace caf
