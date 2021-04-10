#pragma once

#include "cafObjectCapability.h"
#include "cafObjectIoCapability.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <list>
#include <vector>

namespace caf
{
class ObjectIoCapability;
class ObjectHandle;
class ObjectFactory;
class ReferenceHelper;
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
    static void readObjectFromString( ObjectHandle* object, const std::string& string, ObjectFactory* objectFactory );
    static std::string   writeObjectToString( const ObjectHandle* object, bool writeServerAddress );
    static ObjectHandle* copyByJsonSerialization( const ObjectHandle* object, ObjectFactory* objectFactory );
    static ObjectHandle* copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                         const std::string&  destinationClassKeyword,
                                                         const std::string&  sourceClassKeyword,
                                                         ObjectFactory*      objectFactory );

    static ObjectHandle*
        readUnknownObjectFromString( const std::string& string, ObjectFactory* objectFactory, bool isCopyOperation );

    static void readFile( ObjectHandle* object, std::istream& file, ObjectFactory* objectFactory = nullptr );
    static void
        writeFile( const ObjectHandle* object, std::ostream& file, bool writeServerAddress, bool writeValues = true );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    static void readFields( ObjectHandle*         object,
                            const nlohmann::json& jsonObject,
                            ObjectFactory*        objectFactory,
                            bool                  isCopyOperation );
    static void writeFields( const ObjectHandle* object,
                             nlohmann::json&     jsonObject,
                             bool                writeServerAddress,
                             bool                writeValues = true );
};

} // End of namespace caf
