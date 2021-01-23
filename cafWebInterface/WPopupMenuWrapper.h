#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafActionWrapper.h"
#include "cafIconProvider.h"
#include "cafVariant.h"

#include <Wt/WMenuItem.h>

namespace Wt
{
class WPopupMenu;
}

namespace caf
{
class WAction : public ActionWrapper
{
public:
    WAction();
    void init( const std::string& text, void* parent = nullptr ) override;

    Variant          data() const override;
    void             setData( const Variant& variant ) override;
    void             setText( const std::string& text ) override;
    std::string      text() const override;
    void             setEnabled( bool enabled ) override;
    bool             isEnabled() const override;
    void             setChecked( bool checked ) override;
    bool             isChecked() const override;
    bool             isCheckable() const override;
    void             setShortcut( caf::StandardKey keySequence ) override;
    std::string      shortcut() const override;
    bool             isEqualTo( const ActionWrapper* wrapper ) const;
    void             setIcon( const IconProvider& iconProvider ) override;
    IconProvider     icon() const override;
    void             trigger( bool checked ) const override;

    void connect( const std::function<void( bool )>& trigger ) override;
    void actionTriggered( Wt::WMenuItem* menuItem );

private:
    std::string                 m_text;
    Variant                     m_data;
    bool                        m_enabled;
    bool                        m_checked;
    bool                        m_checkable;
    caf::StandardKey            m_shortcut;
    IconProvider                m_icon;
    std::function<void( bool )> m_trigger;
};

class WActionCreator : public ActionCreatorInterface
{
public:
    std::shared_ptr<ActionWrapper> createAction( const std::string& actionName, void* parent = nullptr ) const override;
    static WActionCreator*         instance();
};

class WPopupMenuItem : public Wt::WMenuItem
{
public:
    WPopupMenuItem( const std::string& label, std::shared_ptr<ActionWrapper> action );
    std::shared_ptr<ActionWrapper> action();
    void                           refreshStateFromAction();

private:
    std::shared_ptr<ActionWrapper> m_action;
};

class WPopupMenuWrapper : public MenuInterface
{
public:
    WPopupMenuWrapper();
    ~WPopupMenuWrapper() override;

    Wt::WPopupMenu*                menu();
    MenuInterface*                 addMenu( const IconProvider& icon, const std::string& subMenuName ) override;
    void                           addAction( std::shared_ptr<ActionWrapper> action ) override;
    std::shared_ptr<ActionWrapper> menuAction() const override;
    void                           removeAction( std::shared_ptr<ActionWrapper> action ) override;
    void                           addSeparator() override;
    std::list<std::shared_ptr<ActionWrapper>> actions() const override;

    std::shared_ptr<ActionWrapper> findAction( const std::string& actionName ) const;

    void refreshEnabledState();

private:
    WPopupMenuWrapper( Wt::WPopupMenu* menu );

    Wt::WPopupMenu*                           m_menu;
    std::list<std::shared_ptr<ActionWrapper>> m_actions;
    std::shared_ptr<ActionWrapper>            m_menuAction;
    bool                                      m_takesOwnership;
};
} // namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
