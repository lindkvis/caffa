#include "cafQIconProvider.h"

#include "cafAssert.h"

#include <QApplication>

using namespace caf;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider()
    : m_active(true)
{    
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QString& iconResourceString)
    : m_active(true)
    , m_iconResourceString(iconResourceString)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QPixmap& pixmap)
    : m_iconPixmap(new QPixmap(pixmap))
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QIconProvider& rhs)
    : m_active(rhs.m_active)
    , m_iconResourceString(rhs.m_iconResourceString)
{
    if (rhs.m_icon)
    {
        m_icon = std::make_unique<QIcon>(*rhs.m_icon);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider::QIconProvider(const QIcon& icon)
    : m_active(true)
    , m_icon(std::make_unique<QIcon>(icon))
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIconProvider& QIconProvider::operator=(const QIconProvider& rhs)
{
    if (rhs.m_icon)
    {
        m_icon         = std::make_unique<QIcon>(*rhs.m_icon);
    }
    m_active       = rhs.m_active;
    m_iconResourceString = rhs.m_iconResourceString;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QIcon QIconProvider::icon() const
{
    if (!m_icon)
    {
        m_icon = generateIcon();
    }

    if (!m_active && isGuiApplication())
    {
        QPixmap disabledPixmap = m_icon->pixmap(16, 16, QIcon::Disabled);
        return QIcon(disabledPixmap);
    }

    return *m_icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::isNull() const
{
    if (!isGuiApplication()) return true;

    if (!hasValidPixmap() && m_iconResourceString.isEmpty()) return true;

    return !m_icon || m_icon->isNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setActive(bool active)
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setIconResourceString(const QString& iconResourceString)
{
    m_iconResourceString = iconResourceString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QIconProvider::setPixmap(const QPixmap& pixmap)
{
    m_iconPixmap.reset(new QPixmap(pixmap));
}

//--------------------------------------------------------------------------------------------------
/// Generate the actual icon. Will generate nullptr if a QtGuiApplication isn't running.
/// Override in a sub-class if you want to generate a custom icon procedurally
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> QIconProvider::generateIcon() const
{
    if (isGuiApplication())
    {
        if (hasValidPixmap())
        {
            return std::make_unique<QIcon>(*m_iconPixmap);
        }
        return std::make_unique<QIcon>(m_iconResourceString);
    }
    return std::unique_ptr<QIcon>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::isGuiApplication()
{
    return dynamic_cast<QApplication*>(QCoreApplication::instance()) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QIconProvider::hasValidPixmap() const
{
    return m_iconPixmap && !m_iconPixmap->isNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QIconProvider::iconResourceString() const
{
    return m_iconResourceString;
}
