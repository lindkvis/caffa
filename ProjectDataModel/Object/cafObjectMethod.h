//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafAssert.h"
#include "cafObject.h"
#include "cafObjectFactory.h"
#include "cafPointer.h"

#include <string>

/// CAFFA_OBJECT_METHOD_SOURCE_INIT associates the self class keyword and the method keyword with the method factory
/// Place this in the cpp file, preferably above the constructor
#define CAFFA_OBJECT_METHOD_SOURCE_INIT( SelfClassName, MethodClassName, methodKeyword ) \
    CAFFA_IO_ABSTRACT_SOURCE_INIT( MethodClassName, methodKeyword, "ObjectMethod" )      \
    static bool CAFFA_OBJECT_STRING_CONCATENATE( method##MethodClassName, __LINE__ ) =   \
        caffa::ObjectMethodFactory::instance()->registerMethod<SelfClassName, MethodClassName>()

namespace caffa
{
//==================================================================================================
/// Object script method
/// Sub-class and register to the Object to assign methods to a Object that is accessible from
/// ... scripting engines such as Python.
/// Store arguments as member fields and assign return values in a Object for execute.
/// Return value can be a storage class based on Object returning resultIsPersistent() == false.
/// Or it can be a Object in the project tree returning resultIsPersistent() == true.
///
//==================================================================================================
class ObjectMethod : public Object
{
    CAFFA_HEADER_INIT;

public:
    ObjectMethod( ObjectHandle* self );

    // The returned object contains the results of the method and is the responsibility of the caller.
    virtual ObjectHandle* execute() = 0;

    // Some execute() methods can return a null pointer as a valid return value.
    // Return true here to allow this
    virtual bool isNullptrValidResult() const { return false; }

    virtual std::string selfClassKeyword() const { return m_self->capability<ObjectIoCapability>()->classKeyword(); }

    // True if object is a persistent project tree item. False if the object is to be deleted on completion.
    virtual bool resultIsPersistent() const = 0;

    // In order for the code generators to inspect the fields in the result object any ObjectMethod
    // ... need to provide an implementation that returns the same object type as the execute method.
    virtual std::unique_ptr<ObjectHandle> defaultResult() const = 0;

    // Basically the "this" pointer to the object the method belongs to
    template <typename ObjectType>
    ObjectType* self()
    {
        ObjectType* object = dynamic_cast<ObjectType*>( m_self.p() );
        CAFFA_ASSERT( object );
        return object;
    }

    // Basically the "this" pointer to the object the method belongs to
    template <typename ObjectType>
    const ObjectType* self() const
    {
        const ObjectType* object = dynamic_cast<const ObjectType*>( m_self.p() );
        CAFFA_ASSERT( object );
        return object;
    }

private:
    friend class ObjectScriptingCapability;
    Pointer<ObjectHandle> m_self;
};

//==================================================================================================
/// Object script method factory
/// Register methods with this factory to be able to create and call methods.
//==================================================================================================
class ObjectMethodFactory
{
public:
    static ObjectMethodFactory* instance();

    std::shared_ptr<ObjectMethod> createMethod( ObjectHandle* self, const std::string& methodName );

    template <typename ObjectDerivative, typename ObjectScriptMethodDerivative>
    bool registerMethod()
    {
        std::string className  = ObjectDerivative::classKeywordStatic();
        std::string methodName = ObjectScriptMethodDerivative::classKeywordStatic();

        auto classEntryIt = m_factoryMap.find( className );
        if ( classEntryIt != m_factoryMap.end() )
        {
            auto methodEntryIt = classEntryIt->second.find( methodName );
            if ( methodEntryIt != classEntryIt->second.end() )
            {
                CAFFA_ASSERT( methodName != methodEntryIt->first ); // classNameKeyword has already been used
                CAFFA_ASSERT( false ); // To be sure ..
                return false; // never hit;
            }
        }
        m_factoryMap[className][methodName] =
            std::shared_ptr<ObjectMethodCreatorBase>( new ObjectMethodCreator<ObjectScriptMethodDerivative>() );
        return true;
    }

    std::vector<std::string> registeredMethodNames( const std::string& className ) const;

private:
    ObjectMethodFactory()  = default;
    ~ObjectMethodFactory() = default;

    // Internal helper classes
    class ObjectMethodCreatorBase
    {
    public:
        ObjectMethodCreatorBase() {}
        virtual ~ObjectMethodCreatorBase() {}
        virtual std::shared_ptr<ObjectMethod> create( ObjectHandle* self ) = 0;
    };

    template <typename ObjectScriptMethodDerivative>
    class ObjectMethodCreator : public ObjectMethodCreatorBase
    {
    public:
        std::shared_ptr<ObjectMethod> create( ObjectHandle* self ) override
        {
            return std::shared_ptr<ObjectMethod>( new ObjectScriptMethodDerivative( self ) );
        }
    };

private:
    // Map to store factory
    std::map<std::string, std::map<std::string, std::shared_ptr<ObjectMethodCreatorBase>>> m_factoryMap;
    // Self pointer

}; // namespace caffa

} // namespace caffa
