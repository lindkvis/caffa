// ##################################################################################################
//
//    Custom Visualization Core library
//    Copyright (C) Ceetron Solutions AS
//
//    This library may be used under the terms of either the GNU General Public License or
//    the GNU Lesser General Public License as follows:
//
//    GNU General Public License Usage
//    This library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//    for more details.
//
//    GNU Lesser General Public License Usage
//    This library is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2.1 of the License, or
//    (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or
//    FITNESS FOR A PARTICULAR PURPOSE.
//
//    See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//    for more details.
//
// ##################################################################################################
#pragma once

#include "cafAssert.h"
#include "cafNotNull.h"
#include "cafObject.h"
#include "cafObjectFactory.h"
#include "cafObservingPointer.h"

#include <string>

/// CAFFA_OBJECT_METHOD_SOURCE_INIT associates the self class keyword and the method keyword with the method factory
/// Place this in the cpp file, preferably above the constructor
#define CAFFA_OBJECT_METHOD_SOURCE_INIT( SelfClassName, MethodClassName, methodKeyword ) \
    static bool CAFFA_OBJECT_STRING_CONCATENATE( method##MethodClassName, __LINE__ ) =   \
        caffa::ObjectMethodFactory::instance()->registerMethod<SelfClassName, MethodClassName>( methodKeyword )

namespace caffa
{
//==================================================================================================
/// Object script method result
///
//==================================================================================================
class ObjectMethodResult : public Object
{
    CAFFA_HEADER_INIT( ObjectMethodResult, Object )

public:
    ObjectMethodResult( bool retValue = true, const std::string& errMsg = "" );

    Field<std::string> errorMessage;
    Field<bool>        status;
};

//==================================================================================================
/// Object script method
/// Sub-class and register to the Object to assign methods to a Object that is accessible from
/// ... scripting engines such as Python.
/// Store arguments as member fields and assign return values in a Object for execute.
///
//==================================================================================================
class ObjectMethod : public Object
{
    CAFFA_HEADER_INIT_WITH_DOC( "A generic object method", ObjectMethod, Object )

public:
    enum class Type
    {
        READ_WRITE = 0,
        READ_ONLY
    };

    ObjectMethod( caffa::not_null<ObjectHandle*> self, Type type = Type::READ_WRITE );

    // The returned object contains the results of the method and is the responsibility of the caller.
    virtual std::shared_ptr<ObjectMethodResult> execute() = 0;

    // A default created result object used as a pattern for clients
    virtual std::shared_ptr<ObjectMethodResult> defaultResult() const
    {
        return std::make_shared<ObjectMethodResult>( false );
    }

    // Basically the "this" pointer to the object the method belongs to
    template <typename ObjectType>
    ObjectType* self()
    {
        caffa::not_null<ObjectType*> object = dynamic_cast<ObjectType*>( m_self.p() );
        CAFFA_ASSERT( object );
        return object;
    }

    // Basically the "this" pointer to the object the method belongs to
    template <typename ObjectType>
    const ObjectType* self() const
    {
        caffa::not_null<const ObjectType*> object = dynamic_cast<const ObjectType*>( m_self.p() );
        CAFFA_ASSERT( object );
        return object;
    }

    Type type() const;

private:
    ObservingPointer<ObjectHandle> m_self;

    Type m_type;
};

//==================================================================================================
/// Object script method factory
/// Register methods with this factory to be able to create and call methods.
//==================================================================================================
class ObjectMethodFactory
{
public:
    static ObjectMethodFactory* instance();

    std::unique_ptr<ObjectMethod> createMethodInstance( caffa::not_null<ObjectHandle*> self,
                                                        const std::string_view&        methodName );

    template <typename ObjectDerivative, typename ObjectScriptMethodDerivative>
    bool registerMethod( const std::string& methodName )
    {
        auto className = ObjectDerivative::classKeywordStatic();

        auto classEntryIt = m_factoryMap.find( className );
        if ( classEntryIt != m_factoryMap.end() )
        {
            auto methodEntryIt = classEntryIt->second.find( methodName );
            if ( methodEntryIt != classEntryIt->second.end() )
            {
                CAFFA_ASSERT( methodName != methodEntryIt->first ); // classKeyword has already been used
                CAFFA_ASSERT( false ); // To be sure ..
                return false; // never hit;
            }
        }
        m_factoryMap[std::string( className )][std::string( methodName )] =
            std::shared_ptr<ObjectMethodCreatorBase>( new ObjectMethodCreator<ObjectScriptMethodDerivative>() );
        return true;
    }

    std::vector<std::string> registeredMethodNames( caffa::not_null<const ObjectHandle*> self ) const;

private:
    ObjectMethodFactory()  = default;
    ~ObjectMethodFactory() = default;

    // Internal helper classes
    class ObjectMethodCreatorBase
    {
    public:
        ObjectMethodCreatorBase() {}
        virtual ~ObjectMethodCreatorBase() {}
        virtual std::unique_ptr<ObjectMethod> create( caffa::not_null<ObjectHandle*> self ) = 0;
    };

    template <typename ObjectScriptMethodDerivative>
    class ObjectMethodCreator : public ObjectMethodCreatorBase
    {
    public:
        std::unique_ptr<ObjectMethod> create( caffa::not_null<ObjectHandle*> self ) override
        {
            return std::unique_ptr<ObjectMethod>( new ObjectScriptMethodDerivative( self ) );
        }
    };

private:
    // Map to store factory
    std::map<std::string, std::map<std::string, std::shared_ptr<ObjectMethodCreatorBase>, std::less<>>, std::less<>> m_factoryMap;
    // Self pointer

}; // namespace caffa

} // namespace caffa
