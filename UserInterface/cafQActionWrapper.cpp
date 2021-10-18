#include "cafQActionWrapper.h"

#include "cafAssert.h"
#include "cafQVariantConverter.h"

#include <QAction>
#include <QMenu>
#include <QString>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QActionWrapper::QActionWrapper()
    : m_action( nullptr )
    , m_takesOwnership( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QActionWrapper::QActionWrapper( QAction* action )
    : m_action( action )
    , m_takesOwnership( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QActionWrapper::~QActionWrapper()
{
    if ( m_takesOwnership )
    {
        delete m_action;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::init( const std::string& text, void* parent )
{
    CAFFA_ASSERT( !m_action );
    m_action = new QAction( QString::fromStdString( text ), reinterpret_cast<QObject*>( parent ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* QActionWrapper::action()
{
    return m_action;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QAction* QActionWrapper::action() const
{
    return m_action;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant QActionWrapper::data() const
{
    CAFFA_ASSERT( m_action->data().canConvert<Variant>() );
    return m_action->data().value<Variant>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::setShortcut( StandardKey keySequence )
{
    m_action->setShortcut( static_cast<QKeySequence::StandardKey>( keySequence ) );
#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 ) )
    // Qt made keyboard shortcuts in context menus platform dependent in Qt 5.10
    // With no global way of removing it.
    m_action->setShortcutVisibleInContextMenu( true );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string QActionWrapper::shortcut() const
{
    return m_action->shortcut().toString( QKeySequence::PortableText ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::connect( const std::function<void( bool )>& trigger )
{
    m_trigger = trigger;
    QObject::connect( m_action, SIGNAL( triggered( bool ) ), SLOT( actionTriggered( bool ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::actionTriggered( bool checked )
{
    m_trigger( checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QActionWrapper::isEqualTo( const ActionWrapper* wrapper ) const
{
    auto qWrapper = dynamic_cast<const QActionWrapper*>( wrapper );
    if ( qWrapper )
    {
        return this->action() == qWrapper->action();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::setData( const Variant& variant )
{
    m_action->setData( QVariant::fromValue( variant ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::setText( const std::string& text )
{
    m_action->setText( QString::fromStdString( text ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::setEnabled( bool enabled )
{
    m_action->setEnabled( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QActionWrapper::isEnabled() const
{
    return m_action->isEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::setChecked( bool checked )
{
    m_action->setChecked( checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QActionWrapper::isCheckable() const
{
    return m_action->isCheckable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string QActionWrapper::text() const
{
    return m_action->text().toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QActionWrapper::isChecked() const
{
    return m_action->isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QActionWrapper::trigger( bool checked ) const
{
    m_trigger( checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ActionWrapper> QActionCreator::createAction( const std::string& actionName, void* parent ) const
{
    auto wrapper = std::make_shared<QActionWrapper>();
    wrapper->init( actionName, parent );
    return wrapper;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QActionCreator* QActionCreator::instance()
{
    static QActionCreator* creator = new QActionCreator;
    return creator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenuWrapper::QMenuWrapper()
    : m_menu( new QMenu )
    , m_takesOwnerhip( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenuWrapper::~QMenuWrapper()
{
    if ( m_takesOwnerhip )
    {
        delete m_menu;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenu* QMenuWrapper::menu()
{
    return m_menu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MenuInterface* QMenuWrapper::addMenu( const std::string& subMenuName )
{
    QMenu* subMenu = m_menu->addMenu( QString::fromStdString( subMenuName ) );
    return new QMenuWrapper( subMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMenuWrapper::addAction( std::shared_ptr<ActionWrapper> actionWrapper )
{
    QActionWrapper* qActionWrapper = dynamic_cast<QActionWrapper*>( actionWrapper.get() );
    CAFFA_ASSERT( qActionWrapper );

    m_menu->addAction( qActionWrapper->action() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ActionWrapper> QMenuWrapper::menuAction() const
{
    return std::make_shared<QActionWrapper>( m_menu->menuAction() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMenuWrapper::removeAction( std::shared_ptr<ActionWrapper> actionWrapper )
{
    QActionWrapper* qActionWrapper = dynamic_cast<QActionWrapper*>( actionWrapper.get() );
    CAFFA_ASSERT( qActionWrapper );
    m_menu->removeAction( qActionWrapper->action() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMenuWrapper::addSeparator()
{
    m_menu->addSeparator();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::shared_ptr<ActionWrapper>> QMenuWrapper::actions() const
{
    std::list<std::shared_ptr<ActionWrapper>> wrappers;
    for ( auto action : m_menu->actions() )
    {
        wrappers.push_back( std::make_shared<QActionWrapper>( action ) );
    }
    return wrappers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenuWrapper::QMenuWrapper( QMenu* menu )
    : m_menu( new QMenu )
    , m_takesOwnerhip( false )
{
}
