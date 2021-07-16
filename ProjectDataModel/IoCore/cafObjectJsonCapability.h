#pragma once

#include "cafObjectCapability.h"
#include "cafObjectIoCapability.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <list>
#include <vector>

namespace caffa
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
    static std::string                   writeObjectToString( const ObjectHandle* object );
    static std::unique_ptr<ObjectHandle> copyByJsonSerialization( const ObjectHandle* object, ObjectFactory* objectFactory );
    static std::unique_ptr<ObjectHandle> copyAndCastByJsonSerialization( const ObjectHandle* object,
                                                                         const std::string&  destinationClassKeyword,
                                                                         const std::string&  sourceClassKeyword,
                                                                         ObjectFactory*      objectFactory );

    static std::unique_ptr<ObjectHandle>
        readUnknownObjectFromString( const std::string& string, ObjectFactory* objectFactory, bool copyDataValues );

    static void readFile( ObjectHandle* object, std::istream& file, ObjectFactory* objectFactory = nullptr );
    static void writeFile( const ObjectHandle* object, std::ostream& file, bool copyDataValues = true );

    // Main XML serialization methods that is used internally by the document serialization system
    // Not supposed to be used directly.
    static void readFieldsFromJson( ObjectHandle*         object,
                                    const nlohmann::json& jsonObject,
                                    ObjectFactory*        objectFactory,
                                    bool                  copyDataValues );
    static void writeFieldsToJson( const ObjectHandle* object, nlohmann::json& jsonObject, bool copyDataValues = true );
};

} // End of namespace caffa
