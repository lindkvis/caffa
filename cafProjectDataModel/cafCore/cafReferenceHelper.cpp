//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafReferenceHelper.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafStringTools.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string rootIdentifierString()
{
    return "$ROOT$";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ReferenceHelper::referenceFromRootToObject( ObjectHandle* root, ObjectHandle* obj )
{
    if ( obj == nullptr || root == nullptr ) return std::string();

    std::list<std::string> objectNames = referenceFromRootToObjectAsStringList( root, obj );

    std::string completeReference = caf::StringTools::join( objectNames.begin(), objectNames.end(), " " );

    return completeReference;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ReferenceHelper::referenceFromRootToField( ObjectHandle* root, FieldHandle* field )
{
    if ( field == nullptr || root == nullptr ) return std::string();

    ObjectHandle* owner = field->ownerObject();
    if ( !owner ) return std::string(); // Should be assert ?

    std::list<std::string> refFromRootToField;

    refFromRootToField = referenceFromRootToObjectAsStringList( root, owner );

    refFromRootToField.push_front( field->keyword() );

    std::string completeReference = caf::StringTools::join( refFromRootToField.begin(), refFromRootToField.end(), " " );
    return completeReference;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ReferenceHelper::objectFromReference( ObjectHandle* root, const std::string& reference )
{
    std::list<std::string> decodedReference = caf::StringTools::split( reference, " " );

    return objectFromReferenceStringList( root, decodedReference );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ReferenceHelper::findField( ObjectHandle* object, const std::string& fieldKeyword )
{
    if ( object == nullptr ) return nullptr;

    std::vector<FieldHandle*> fields;
    object->fields( fields );

    for ( size_t i = 0; i < fields.size(); i++ )
    {
        if ( fields[i]->keyword() == fieldKeyword )
        {
            return fields[i];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::string> ReferenceHelper::referenceFromRootToObjectAsStringList( ObjectHandle* root, ObjectHandle* obj )
{
    std::list<std::string> objectNames;

    if ( obj != nullptr && root )
    {
        if ( obj == root ) return objectNames;

        ObjectHandle* currentObject = obj;

        bool continueParsing = true;
        while ( continueParsing )
        {
            caf::FieldHandle* parentField = currentObject->parentField();
            if ( !parentField )
            {
                // Could not find a path from obj to root, obj and root are unrelated objects
                return std::list<std::string>();
            }

            std::vector<ObjectHandle*> childObjects;
            parentField->childObjects( &childObjects );

            if ( childObjects.size() > 0 )
            {
                int index = -1;

                for ( size_t i = 0; i < childObjects.size(); i++ )
                {
                    if ( childObjects[i] == currentObject )
                    {
                        index = static_cast<int>( i );
                    }
                }

                objectNames.push_front( std::to_string( index ) );
                objectNames.push_front( parentField->keyword() );
            }
            else
            {
                continueParsing = false;
                continue;
            }

            ObjectHandle* ownerObject = parentField->ownerObject();
            if ( !ownerObject )
            {
                // Could not find a path from obj to root, obj and root are unrelated objects
                return std::list<std::string>();
            }

            if ( ownerObject == root )
            {
                continueParsing = false;
                continue;
            }

            currentObject = ownerObject;
        }
    }

    return objectNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle* ReferenceHelper::fieldFromReference( ObjectHandle* root, const std::string& reference )
{
    std::list<std::string> decodedReference = caf::StringTools::split( reference, " " );
    if ( decodedReference.size() == 0 ) return nullptr;

    std::string fieldKeyword = decodedReference.front();
    decodedReference.pop_front();

    ObjectHandle* parentObject = objectFromReferenceStringList( root, decodedReference );
    return findField( parentObject, fieldKeyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ReferenceHelper::objectFromReferenceStringList( ObjectHandle* root, const std::list<std::string>& reference )
{
    if ( !root ) return nullptr;

    ObjectHandle* currentObject = root;

    auto it = reference.begin();
    while ( it != reference.end() )
    {
        std::string fieldKeyword = *it++;

        FieldHandle* fieldHandle = findField( currentObject, fieldKeyword );
        if ( !fieldHandle )
        {
            return nullptr;
        }

        std::vector<ObjectHandle*> childObjects;
        fieldHandle->childObjects( &childObjects );

        if ( childObjects.size() == 0 )
        {
            return nullptr;
        }

        std::string fieldIndex = *it++;

        int index = std::numeric_limits<int>::infinity();
        try
        {
            index = std::stoi( fieldIndex );
        }
        catch ( std::exception& )
        {
            return nullptr;
        }

        if ( index < 0 || index > ( (int)childObjects.size() ) - 1 )
        {
            return nullptr;
        }

        currentObject = childObjects[index];
    }

    return currentObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<ObjectHandle*> findPathToObjectFromRoot( ObjectHandle* obj )
{
    std::vector<ObjectHandle*> objPath;
    ObjectHandle*              currentObj = obj;
    while ( currentObj )
    {
        objPath.push_back( currentObj );
        if ( currentObj->parentField() )
        {
            currentObj = currentObj->parentField()->ownerObject();
        }
        else
        {
            currentObj = nullptr;
        }
    }

    std::reverse( objPath.begin(), objPath.end() );

    return objPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ReferenceHelper::referenceFromFieldToObject( FieldHandle* fromField, ObjectHandle* toObj )
{
    if ( !fromField || !toObj ) return "";

    ObjectHandle* fromObj = fromField->ownerObject();
    if ( !fromObj ) return "";

    std::vector<ObjectHandle*> fromObjPath = findPathToObjectFromRoot( fromObj );
    std::vector<ObjectHandle*> toObjPath   = findPathToObjectFromRoot( toObj );

    if ( fromObjPath.empty() || toObjPath.empty() ) return std::string();

    // Make sure the objects actually have at least one common ancestor
    if ( fromObjPath.front() != toObjPath.front() ) return std::string();

    bool   anchestorIsEqual         = true;
    size_t idxToLastCommonAnchestor = 0;
    while ( anchestorIsEqual )
    {
        ++idxToLastCommonAnchestor;
        if ( idxToLastCommonAnchestor >= fromObjPath.size() || idxToLastCommonAnchestor >= toObjPath.size() ||
             fromObjPath[idxToLastCommonAnchestor] != toObjPath[idxToLastCommonAnchestor] )
        {
            anchestorIsEqual = false;
            idxToLastCommonAnchestor -= 1;
        }
    }

    size_t levelCountToCommonAnchestor = ( fromObjPath.size() - 1 ) - idxToLastCommonAnchestor;

    ObjectHandle* lastCommonAnchestor = fromObjPath[idxToLastCommonAnchestor];

    std::list<std::string> referenceList = referenceFromRootToObjectAsStringList( lastCommonAnchestor, toObj );

    if ( idxToLastCommonAnchestor == 0 )
    {
        referenceList.push_front( rootIdentifierString() );
    }
    else
    {
        for ( size_t i = 0; i < levelCountToCommonAnchestor; ++i )
        {
            referenceList.push_front( ".." );
        }
    }

    std::string completeReference = caf::StringTools::join( referenceList.begin(), referenceList.end(), " " );

    return completeReference;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ReferenceHelper::objectFromFieldReference( FieldHandle* fromField, const std::string& reference )
{
    if ( !fromField ) return nullptr;
    if ( reference.empty() ) return nullptr;

    if ( caf::StringTools::trim( reference ).empty() ) return nullptr;

    std::list<std::string> decodedReference    = caf::StringTools::split( reference, std::regex( "\\s+" ), true );
    ObjectHandle*          lastCommonAnchestor = fromField->ownerObject();
    CAF_ASSERT( lastCommonAnchestor );

    if ( !decodedReference.empty() && decodedReference.front() == rootIdentifierString() )
    {
        lastCommonAnchestor = findRoot( lastCommonAnchestor );
        decodedReference.pop_front();
    }
    else
    {
        while ( !decodedReference.empty() && decodedReference.front() == ".." )
        {
            FieldHandle* parentField = lastCommonAnchestor->parentField();
            if ( !parentField )
            {
                // Error: Relative object reference has an invalid number of parent levels
                return nullptr;
            }

            lastCommonAnchestor = parentField->ownerObject();
            CAF_ASSERT( lastCommonAnchestor );
            decodedReference.pop_front();
        }
    }

    return objectFromReferenceStringList( lastCommonAnchestor, decodedReference );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ReferenceHelper::findRoot( ObjectHandle* obj )
{
    std::vector<ObjectHandle*> path = findPathToObjectFromRoot( obj );

    if ( !path.empty() )
        return path.front();
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectHandle* ReferenceHelper::findRoot( FieldHandle* field )
{
    if ( field )
    {
        ObjectHandle* ownerObject = field->ownerObject();
        return findRoot( ownerObject );
    }

    return nullptr;
}

} // end namespace caf
