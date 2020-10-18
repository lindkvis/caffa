#pragma once

#include "cafPdmUiCore_export.h"

#include "cafIconProvider.h"

#include <functional>
#include <memory>

#include <QObject>
#include <QKeySequence>
#include <QString>
#include <QVariant>

namespace caf
{
class cafPdmUiCore_EXPORT ActionWrapper : public QObject
{
    Q_OBJECT;

public:
    virtual ~ActionWrapper() = default;

    virtual void         init( const QString& actionName, QObject* parent = nullptr ) = 0;
    virtual QVariant     data() const                                                 = 0;
    virtual void         setData( const QVariant& variant )                           = 0;
    virtual void         setText( const QString& text )                               = 0;
    virtual QString      text() const                                                 = 0;
    virtual void         setEnabled( bool enabled )                                   = 0;
    virtual bool         isEnabled() const                                            = 0;
    virtual void         setChecked( bool checked )                                   = 0;
    virtual bool         isChecked() const                                            = 0;
    virtual bool         isCheckable() const                                          = 0;
    virtual QKeySequence shortcut() const                                             = 0;
    virtual void         setShortcut( QKeySequence::StandardKey shortcut )            = 0;
    virtual bool         isEqualTo( const ActionWrapper* wrapper ) const              = 0;
    virtual void         setIcon( const IconProvider& iconProvider )                  = 0;
    virtual IconProvider icon() const                                                 = 0;
    virtual void         trigger( bool checked ) const                                = 0;
    virtual void         connect( const std::function<void( bool )>& trigger )        = 0;

};

class ActionCreatorInterface
{
public:
    virtual std::shared_ptr<ActionWrapper> createAction( const QString& actionName, QObject* parent = nullptr ) const = 0;
};

class MenuInterface
{
public:
    virtual ~MenuInterface()                                                                               = default;
    virtual MenuInterface*                 addMenu( const IconProvider& icon, const QString& subMenuName ) = 0;
    virtual void                           addAction( std::shared_ptr<ActionWrapper> actionWrapper )       = 0;
    virtual std::shared_ptr<ActionWrapper> menuAction() const                                              = 0;
    virtual void                           removeAction( std::shared_ptr<ActionWrapper> actionWrapper )    = 0;
    virtual void                           addSeparator()                                                  = 0;
    virtual std::list<std::shared_ptr<ActionWrapper>> actions() const                                      = 0;
};

} // namespace caf
