
#include "cafUiTabbedPropertyViewDialog.h"

#include "cafObject.h"
#include "cafUiPropertyView.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTabbedPropertyViewDialog::UiTabbedPropertyViewDialog( caffa::Object*       object,
                                                        const QStringList& tabLabels,
                                                        const QString&     windowTitle,
                                                        QWidget*           parent )
    : QDialog( parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint )
{
    this->setWindowTitle( windowTitle );

    QTabWidget* tabWidget = new QTabWidget;

    for ( int i = 0; i < tabLabels.size(); i++ )
    {
        QHBoxLayout* widgetLayout = new QHBoxLayout;
        widgetLayout->setContentsMargins( 0, 0, 0, 0 );

        QWidget* containerWidget = new QWidget;
        containerWidget->setLayout( widgetLayout );

        caffa::UiPropertyView* uiPropertyView = new caffa::UiPropertyView();

        widgetLayout->addWidget( uiPropertyView );

        tabWidget->addTab( containerWidget, tabLabels[i] );
        uiPropertyView->showProperties( object );

        m_propertyViewTabs.push_back( uiPropertyView );
    }

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout( dialogLayout );

    dialogLayout->addWidget( tabWidget );

    m_dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    connect( m_dialogButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( m_dialogButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    dialogLayout->addWidget( m_dialogButtonBox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTabbedPropertyViewDialog::~UiTabbedPropertyViewDialog()
{
    for ( auto propView : m_propertyViewTabs )
    {
        propView->showProperties( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize UiTabbedPropertyViewDialog::minimumSizeHint() const
{
    QSize minSizeHint( 0, 0 );

    for ( auto propView : m_propertyViewTabs )
    {
        QSize pageSize = propView->minimumSizeHint();
        pageSize += QSize( 0, 100 );

        minSizeHint = minSizeHint.expandedTo( pageSize );
    }

    return minSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize UiTabbedPropertyViewDialog::sizeHint() const
{
    QSize maxSizeHint( 0, 0 );

    for ( auto w : m_propertyViewTabs )
    {
        QSize pageSize = w->sizeHint();
        pageSize += QSize( 0, 100 );

        maxSizeHint = maxSizeHint.expandedTo( pageSize );
    }

    return maxSizeHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDialogButtonBox* UiTabbedPropertyViewDialog::dialogButtonBox()
{
    return m_dialogButtonBox;
}

} // namespace caffa
