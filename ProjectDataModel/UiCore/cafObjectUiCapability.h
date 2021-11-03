#pragma once

#include "cafObjectCapability.h"
#include "cafUiItem.h"

#include <list>
#include <string>

namespace caffa
{
class MenuInterface;
class UiEditorAttribute;
class UiTreeOrdering;
class ObjectHandle;
class UiOrdering;
class FieldHandle;
class UiEditorAttribute;

class ObjectUiCapability : public UiItem, public ObjectCapability
{
public:
    ObjectUiCapability( ObjectHandle* owner, bool giveOwnership );
    ~ObjectUiCapability() noexcept override;

    ObjectHandle*       objectHandle() { return m_owner; }
    const ObjectHandle* objectHandle() const { return m_owner; }

    /// Method to be called from the Ui classes creating Auto Gui to get the group information
    /// supplied by the \sa defineUiOrdering method that can be reimplemented
    void uiOrdering( UiOrdering& uiOrdering );

    /// Method to be called by Ui displaying a tree representation of the object hierarchy
    /// Caller must delete the returned object.
    UiTreeOrdering* uiTreeOrdering() const;
    /// Helper function to expand a pre-defined tree start
    static void expandUiTree( UiTreeOrdering* root );

    /// For a specific field, return editor specific parameters used to customize the editor behavior.
    void editorAttribute( const FieldHandle* field, UiEditorAttribute* attribute );

    /// Return object editor specific parameters used to customize the editor behavior.
    void objectEditorAttribute( UiEditorAttribute* attribute );

    /// Field used to control if field change of and object should be covered by undo/redo framework
    virtual bool useUndoRedoForFieldChanged() { return true; }

    // Virtual interface to override in subclasses to support special behaviour if needed
public: // Virtual
    void fieldChangedByUi( const FieldHandle* changedField, const Variant& oldValue, const Variant& newValue );

    /// Method to re-implement to supply option values for a specific field
    virtual std::deque<OptionItemInfo> calculateValueOptions( const caffa::FieldHandle* fieldNeedingOptions,
                                                              bool*                     useOptionsOnly )
    {
        return std::deque<OptionItemInfo>();
    }

    /// Override used to attach application defined slots to caf created widgets
    /// All field editor widgets are supposed to be created when this function is called
    virtual void onEditorWidgetsCreated() {}

protected:
    /// Override to customize the order and grouping of the Gui.
    /// Fill up the uiOrdering object with groups and field references to create the gui structure
    /// If the uiOrdering is empty, it is interpreted as meaning all fields w/o grouping.
    virtual void defineUiOrdering( caffa::UiOrdering& uiOrdering ) {}

    /// Override to customize the tree representations of the object hierarchy.
    /// If the UiTreeOrdering is empty, it is interpreted as meaning all fields containing child objects in order
    virtual void defineUiTreeOrdering( caffa::UiTreeOrdering& uiTreeOrdering ) {}

    /// Override to provide editor specific data for the field and
    virtual void defineEditorAttribute( const caffa::FieldHandle* field, caffa::UiEditorAttribute* attribute ) {}

    /// Override to provide editor specific data for the  for the object
    virtual void defineObjectEditorAttribute( caffa::UiEditorAttribute* attribute ) {}

private:
    /// Helper method for the TreeItem generation stuff
    void addDefaultUiTreeChildren( UiTreeOrdering* uiTreeOrdering );

    ObjectHandle* m_owner;
};

ObjectUiCapability* uiObj( const ObjectHandle* obj );

} // End of namespace caffa
