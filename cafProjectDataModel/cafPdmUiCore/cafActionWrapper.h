#pragma once

#include "cafIconProvider.h"
#include "cafVariant.h"

#include <functional>
#include <memory>
#include <string>

namespace caf
{
// Lifted from Qt to exactly match the Qt-standardkeys
enum StandardKey
{
    UnknownKey,
    HelpContents,
    WhatsThis,
    Open,
    Close,
    Save,
    New,
    Delete,
    Cut,
    Copy,
    Paste,
    Undo,
    Redo,
    Back,
    Forward,
    Refresh,
    ZoomIn,
    ZoomOut,
    Print,
    AddTab,
    NextChild,
    PreviousChild,
    Find,
    FindNext,
    FindPrevious,
    Replace,
    SelectAll,
    Bold,
    Italic,
    Underline,
    MoveToNextChar,
    MoveToPreviousChar,
    MoveToNextWord,
    MoveToPreviousWord,
    MoveToNextLine,
    MoveToPreviousLine,
    MoveToNextPage,
    MoveToPreviousPage,
    MoveToStartOfLine,
    MoveToEndOfLine,
    MoveToStartOfBlock,
    MoveToEndOfBlock,
    MoveToStartOfDocument,
    MoveToEndOfDocument,
    SelectNextChar,
    SelectPreviousChar,
    SelectNextWord,
    SelectPreviousWord,
    SelectNextLine,
    SelectPreviousLine,
    SelectNextPage,
    SelectPreviousPage,
    SelectStartOfLine,
    SelectEndOfLine,
    SelectStartOfBlock,
    SelectEndOfBlock,
    SelectStartOfDocument,
    SelectEndOfDocument,
    DeleteStartOfWord,
    DeleteEndOfWord,
    DeleteEndOfLine,
    InsertParagraphSeparator,
    InsertLineSeparator,
    SaveAs,
    Preferences,
    Quit,
    FullScreen,
    Deselect,
    DeleteCompleteLine,
    Backspace,
    Cancel
};

class ActionWrapper
{
public:
    virtual ~ActionWrapper() = default;

    virtual void         init( const std::string& actionName, void* parent = nullptr ) = 0;
    virtual Variant      data() const                                                  = 0;
    virtual void         setData( const Variant& variant )                             = 0;
    virtual void         setText( const std::string& text )                            = 0;
    virtual std::string  text() const                                                  = 0;
    virtual void         setEnabled( bool enabled )                                    = 0;
    virtual bool         isEnabled() const                                             = 0;
    virtual void         setChecked( bool checked )                                    = 0;
    virtual bool         isChecked() const                                             = 0;
    virtual bool         isCheckable() const                                           = 0;
    virtual std::string  shortcut() const                                              = 0;
    virtual void         setShortcut( StandardKey shortcut )                           = 0;
    virtual bool         isEqualTo( const ActionWrapper* wrapper ) const               = 0;
    virtual void         setIcon( const IconProvider& iconProvider )                   = 0;
    virtual IconProvider icon() const                                                  = 0;
    virtual void         trigger( bool checked ) const                                 = 0;
    virtual void         connect( const std::function<void( bool )>& trigger )         = 0;
};

class ActionCreatorInterface
{
public:
    virtual std::shared_ptr<ActionWrapper> createAction( const std::string& actionName, void* parent = nullptr ) const = 0;
};

class MenuInterface
{
public:
    virtual ~MenuInterface() = default;
    virtual MenuInterface*                 addMenu( const IconProvider& icon, const std::string& subMenuName ) = 0;
    virtual void                           addAction( std::shared_ptr<ActionWrapper> actionWrapper )           = 0;
    virtual std::shared_ptr<ActionWrapper> menuAction() const                                                  = 0;
    virtual void                           removeAction( std::shared_ptr<ActionWrapper> actionWrapper )        = 0;
    virtual void                           addSeparator()                                                      = 0;
    virtual std::list<std::shared_ptr<ActionWrapper>> actions() const                                          = 0;
};

} // namespace caf
