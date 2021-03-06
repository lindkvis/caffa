#pragma once

#include "cafActionWrapper.h"

#include <QObject>

class QAction;
class QMenu;

namespace caffa
{
class QActionWrapper : public QObject, public ActionWrapper
{
    Q_OBJECT;

public:
    QActionWrapper();
    QActionWrapper( QAction* action );
    ~QActionWrapper() override;

    void init( const std::string& actionName, void* parent = nullptr ) override;

    QAction*       action();
    const QAction* action() const;
    Variant        data() const override;
    void           setData( const Variant& variant ) override;
    void           setText( const std::string& text ) override;
    std::string    text() const override;
    void           setEnabled( bool enabled ) override;
    bool           isEnabled() const override;
    void           setChecked( bool checked ) override;
    bool           isChecked() const override;
    bool           isCheckable() const override;
    void           setShortcut( StandardKey shortcut ) override;
    std::string    shortcut() const override;
    bool           isEqualTo( const ActionWrapper* wrapper ) const;
    void           setIcon( const IconProvider& iconProvider ) override;
    IconProvider   icon() const override;
    void           trigger( bool checked ) const override;

    void connect( const std::function<void( bool )>& trigger ) override;

private slots:
    void actionTriggered( bool checked );

private:
    QAction*                    m_action;
    IconProvider                m_iconProvider;
    bool                        m_takesOwnership;
    std::function<void( bool )> m_trigger;
};

class QActionCreator : public ActionCreatorInterface
{
public:
    std::shared_ptr<ActionWrapper> createAction( const std::string& actionName, void* parent = nullptr ) const override;
    static QActionCreator*         instance();
};

class QMenuWrapper : public MenuInterface
{
public:
    QMenuWrapper();
    ~QMenuWrapper() override;

    QMenu* menu();

    MenuInterface*                 addMenu( const IconProvider& iconProvider, const std::string& subMenuName ) override;
    void                           addAction( std::shared_ptr<ActionWrapper> actionWrapper ) override;
    std::shared_ptr<ActionWrapper> menuAction() const override;
    void                           removeAction( std::shared_ptr<ActionWrapper> actionWrapper ) override;
    void                           addSeparator() override;
    std::list<std::shared_ptr<ActionWrapper>> actions() const override;

private:
    QMenuWrapper( QMenu* menu );

    QMenu* m_menu;
    bool   m_takesOwnerhip;
};

} // namespace caffa
