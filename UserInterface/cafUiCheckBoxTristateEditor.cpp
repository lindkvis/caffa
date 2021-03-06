

#include "cafUiCheckBoxTristateEditor.h"

#include "cafUiDefaultObjectEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafFactory.h"
#include "cafTristate.h"

#include <QLabel>

namespace caffa
{
CAFFA_UI_FIELD_EDITOR_SOURCE_INIT( UiCheckBoxTristateEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCheckBoxTristateEditor::configureAndUpdateUi()
{
    CAFFA_ASSERT( !m_checkBox.isNull() );
    CAFFA_ASSERT( !m_label.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_checkBox->setEnabled( !uiField()->isUiReadOnly() );
    m_checkBox->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    Tristate state = uiField()->uiValue().value<Tristate>();

    if ( state == Tristate::State::True )
    {
        m_checkBox->setCheckState( Qt::Checked );
    }
    else if ( state == Tristate::State::PartiallyTrue )
    {
        m_checkBox->setCheckState( Qt::PartiallyChecked );
    }
    else if ( state == Tristate::State::False )
    {
        m_checkBox->setCheckState( Qt::Unchecked );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiCheckBoxTristateEditor::createEditorWidget( QWidget* parent )
{
    m_checkBox = new QCheckBox( parent );
    m_checkBox->setTristate( true );

    connect( m_checkBox, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiCheckBoxTristateEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCheckBoxTristateEditor::slotClicked( bool )
{
    Tristate state;

    if ( m_checkBox->checkState() == Qt::Checked )
    {
        state = Tristate::State::True;
    }
    else if ( m_checkBox->checkState() == Qt::PartiallyChecked )
    {
        state = Tristate::State::PartiallyTrue;
    }
    else if ( m_checkBox->checkState() == Qt::Unchecked )
    {
        state = Tristate::State::False;
    }

    Variant v( state );

    this->setValueToField( v );
}

} // end namespace caffa
