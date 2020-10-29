#pragma once

#include "cafObjectCapability.h"
#include "cafObjectIoCapability.h"

#include <QString>

#include <list>
#include <vector>

class QIODevice;
class QJsonObject;

namespace caf
{
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
class ObjectJsonCapability
{
public:
    /// Convenience methods to serialize/de-serialize this particular object (with children)
    static void readObjectFromString( ObjectHandle* object, const QString& string, ObjectFactory* objectFactory );
    static QString          writeObjectToString( const ObjectHandle* object );
    static ObjectHandle* copyByJsonSerialization( const ObjectHandle* object, ObjectFactory* objectFactory );
    static ObjectHandle* copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                            const QString&         destinationClassKeyword,
                                                            const QString&         sourceClassKeyword,
                                                            ObjectFactory*      objectFactory );

    static ObjectHandle*
        readUnknownObjectFromString( const QString& string, ObjectFactory* objectFactory, bool isCopyOperation );

    static void readFile( ObjectHandle* object, QIODevice* file );
    static void writeFile( const ObjectHandle* object, QIODevice* file );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    static void readFields( ObjectHandle*   object,
                            const QJsonObject& jsonObject,
                            ObjectFactory*  objectFactory,
                            bool               isCopyOperation );
    static void writeFields( const ObjectHandle* object, QJsonObject& jsonObject );
};

} // End of namespace caf
