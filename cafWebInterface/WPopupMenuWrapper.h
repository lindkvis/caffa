#pragma once

#include "cafActionWrapper.h"
#include "cafQIconProvider.h"

#include <Wt/WMenuItem.h>

#include <QKeySequence>
#include <QList>
#include <QVariant>

class QString;

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
    void init(const QString& text, QObject* parent = nullptr) override;

    QVariant data() const override;
    void setData(const QVariant& variant) override;
    void setText(const QString& text) override;
    QString text() const override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    void setChecked(bool checked) override;
    bool isChecked() const override;
    bool isCheckable() const override;
    void setShortcut(QKeySequence::StandardKey keySequence) override;
    QKeySequence shortcut() const override;
    bool isEqualTo(const ActionWrapper* wrapper) const;
    void setIcon(const QIconProvider& iconProvider) override;
    QIconProvider icon() const override;
    void trigger(bool checked) const override;

    void connect(const std::function<void(bool) >& trigger) override;
    void actionTriggered(Wt::WMenuItem* menuItem);
private:
    QString                   m_text;
    QVariant                  m_data;
    bool                      m_enabled;
    bool                      m_checked;
    bool                      m_checkable;
    QKeySequence::StandardKey m_shortcut;
    QIconProvider             m_icon;
    std::function<void(bool)> m_trigger;
};

class WActionCreator : public ActionCreatorInterface
{
public:
    std::shared_ptr<ActionWrapper> createAction(const QString& actionName, QObject* parent = nullptr) const override;
    static WActionCreator* instance();
};

class WPopupMenuItem : public Wt::WMenuItem
{
public:
    WPopupMenuItem(const QString& label, std::shared_ptr<ActionWrapper> action);
    std::shared_ptr<ActionWrapper> action();
    void refreshStateFromAction();
private:
    std::shared_ptr<ActionWrapper> m_action;
};

class WPopupMenuWrapper : public MenuInterface
{

public:
    WPopupMenuWrapper();
    ~WPopupMenuWrapper() override;

    Wt::WPopupMenu* menu();
    MenuInterface* addMenu(const QIconProvider& icon, const QString& subMenuName) override;
    void addAction(std::shared_ptr<ActionWrapper> action) override;
    std::shared_ptr<ActionWrapper> menuAction() const override;
    void removeAction(std::shared_ptr<ActionWrapper> action) override;
    void addSeparator() override;
    std::list<std::shared_ptr<ActionWrapper>> actions() const override;

    std::shared_ptr<ActionWrapper> findAction(const QString& actionName) const;
    
    void refreshEnabledState();
private:
    WPopupMenuWrapper(Wt::WPopupMenu* menu);

    Wt::WPopupMenu*                           m_menu;
    std::list<std::shared_ptr<ActionWrapper>> m_actions;
    std::shared_ptr<ActionWrapper>            m_menuAction;
    bool                                      m_takesOwnership;
};
}
