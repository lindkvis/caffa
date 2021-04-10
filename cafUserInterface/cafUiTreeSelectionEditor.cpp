//##################################################################################################
//
//   Custom Visualization Core library
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafUiTreeSelectionEditor.h"

#include "cafAssert.h"
#include "cafObject.h"
#include "cafQVariantConverter.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiTreeSelectionQModel.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSortFilterProxyModel>
#include <QStyleOption>
#include <QTimer>
#include <QTreeView>

#include <algorithm>

//==================================================================================================
/// Helper class used to control height of size hint
//==================================================================================================
class QTreeViewHeightHint : public QTreeView
{
public:
    explicit QTreeViewHeightHint( QWidget* parent = nullptr )
        : m_heightHint( -1 )
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QSize sizeHint() const override
    {
        QSize mySize = QTreeView::sizeHint();

        if ( m_heightHint > 0 )
        {
            mySize.setHeight( m_heightHint );
        }

        return mySize;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void setHeightHint( int heightHint ) { m_heightHint = heightHint; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void keyPressEvent( QKeyEvent* event ) override
    {
        QTreeView::keyPressEvent( event );

        if ( event->key() == Qt::Key_Down || event->key() == Qt::Key_Up || event->key() == Qt::Key_Home ||
             event->key() == Qt::Key_End || event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp )
        {
            emit clicked( currentIndex() );
        }
    }

private:
    int m_heightHint;
};

//==================================================================================================
///
//==================================================================================================
class FilterLeafNodesOnlyProxyModel : public QSortFilterProxyModel
{
public:
    FilterLeafNodesOnlyProxyModel( QObject* parent = nullptr )
        : QSortFilterProxyModel( parent )
    {
    }

protected:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const override
    {
        QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

        if ( sourceModel()->hasChildren( index ) )
        {
            // Always include node if node has children
            return true;
        }

        return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
    }
};

//--------------------------------------------------------------------------------------------------
/// Ported parts of placeholder text painting from Qt 4.7
/// setPlaceholderText() was introduced in Qt 4.7,
/// and this class is intended to be removed when Qt is upgraded
//--------------------------------------------------------------------------------------------------
class PlaceholderLineEdit : public QLineEdit
{
public:
    explicit PlaceholderLineEdit( const QString& placeholderText, QWidget* parent = nullptr )
        : QLineEdit( parent )
        , m_placeholderText( placeholderText )
    {
    }

private:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void paintEvent( QPaintEvent* paintEvent ) override
    {
        QPainter p( this );

        QRect    r   = rect();
        QPalette pal = palette();

#if QT_VERSION_MAJOR > 4
        QStyleOptionFrame panel;
#else
        QStyleOptionFrameV2 panel;
#endif
        initStyleOption( &panel );
        style()->drawPrimitive( QStyle::PE_PanelLineEdit, &panel, &p, this );
        r = style()->subElementRect( QStyle::SE_LineEditContents, &panel, this );

        QMargins margins = textMargins();

        r.setX( r.x() + margins.left() );
        r.setY( r.y() + margins.top() );
        r.setRight( r.right() - margins.right() );
        r.setBottom( r.bottom() - margins.bottom() );
        p.setClipRect( r );

        QFontMetrics fm = fontMetrics();

        const int horizontalMargin = 2;
        const int vscroll          = r.y() + ( r.height() - fm.height() + 1 ) / 2;

        QRect lineRect( r.x() + horizontalMargin, vscroll, r.width() - 2 * horizontalMargin, fm.height() );

        int minLB = qMax( 0, -fm.minLeftBearing() );

        if ( text().isEmpty() )
        {
            if ( !hasFocus() && !m_placeholderText.isEmpty() )
            {
                QColor col = pal.text().color();
                col.setAlpha( 128 );
                QPen oldpen = p.pen();
                p.setPen( col );
                lineRect.adjust( minLB, 0, 0, 0 );
                QString elidedText = fm.elidedText( m_placeholderText, Qt::ElideRight, lineRect.width() );
                p.drawText( lineRect, Qt::AlignLeft | Qt::TextWordWrap, elidedText );
                p.setPen( oldpen );

                return;
            }
        }

        return QLineEdit::paintEvent( paintEvent );
    }

private:
    QString m_placeholderText;
};

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiTreeSelectionEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeSelectionEditor::UiTreeSelectionEditor()
    : m_model( nullptr )
    , m_proxyModel( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeSelectionEditor::~UiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::configureAndUpdateUi()
{
    // Label
    CAF_ASSERT( !m_label.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    if ( !m_model )
    {
        m_model = new caf::UiTreeSelectionQModel( this );

        m_proxyModel = new FilterLeafNodesOnlyProxyModel( this );
        m_proxyModel->setSourceModel( m_model );
        m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

        m_treeView->setModel( m_proxyModel );
    }

    bool optionsOnly = true;
    auto options     = uiField()->valueOptions( &optionsOnly );

    bool itemCountHasChaged = false;
    if ( m_model->optionItemCount() != options.size() ) itemCountHasChaged = true;

    Variant fieldValue = uiField()->uiValue();
    m_model->setUiValueCache( fieldValue );

    // TODO: If the count is different between incoming and current list of items,
    // use cafQTreeViewStateSerializer to restore collapsed state
    m_model->setOptions( this, options );

    if ( itemCountHasChaged )
    {
        m_treeView->expandAll();
    }

    if ( !fieldValue.isVector() )
    {
        m_textFilterLineEdit->hide();
        m_toggleAllCheckBox->hide();
    }
    else
    {
        caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
        if ( uiObject )
        {
            uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
        }

        if ( m_attributes.singleSelectionMode )
        {
            m_treeView->setSelectionMode( QAbstractItemView::SingleSelection );
            m_treeView->setContextMenuPolicy( Qt::NoContextMenu );

            m_model->enableSingleSelectionMode( m_attributes.singleSelectionMode );
        }
        else
        {
            m_treeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
        }

        connect( m_treeView, SIGNAL( clicked( QModelIndex ) ), this, SLOT( slotClicked( QModelIndex ) ), Qt::UniqueConnection );

        if ( !m_attributes.showTextFilter )
        {
            m_textFilterLineEdit->hide();
        }

        if ( m_attributes.singleSelectionMode || !m_attributes.showToggleAllCheckbox )
        {
            m_toggleAllCheckBox->hide();
        }
        else
        {
            if ( options.size() == 0 )
            {
                m_toggleAllCheckBox->setChecked( false );
            }
            else
            {
                QModelIndexList indices = allVisibleSourceModelIndices();
                if ( indices.size() > 0 )
                {
                    size_t editableItems        = 0u;
                    size_t checkedEditableItems = 0u;
                    for ( auto mi : indices )
                    {
                        if ( !m_model->isReadOnly( mi ) )
                        {
                            editableItems++;
                            if ( m_model->isChecked( mi ) )
                            {
                                checkedEditableItems++;
                            }
                        }
                    }
                    bool allItemsChecked = ( editableItems > 0u && checkedEditableItems == editableItems );
                    m_toggleAllCheckBox->setChecked( allItemsChecked );
                }
            }
        }
    }

    // If the tree doesn't have grand children we treat this as a straight list
    m_treeView->setRootIsDecorated( m_model->hasGrandChildren() );

    m_model->resetUiValueCache();

    if ( m_attributes.currentIndexFieldHandle )
    {
        FieldUiCapability* uiFieldHandle = m_attributes.currentIndexFieldHandle->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            QModelIndexList indices          = allVisibleSourceModelIndices();
            Variant         currentItemValue = uiFieldHandle->uiValue();

            for ( const auto& mi : indices )
            {
                QVariant qItemValue = m_model->data( mi, UiTreeSelectionQModel::optionItemValueRole() );
                Variant  itemValue  = qItemValue.value<Variant>();
                if ( currentItemValue == itemValue )
                {
                    QModelIndex treeViewIndex = m_proxyModel->mapFromSource( mi );
                    m_treeView->setCurrentIndex( treeViewIndex );
                }
            }
        }
    }

    // It is required to use a timer here, as the layout of the widgets are handled by events
    // Calling scrollTo() here has no effect, or scrolls to wrong location
    QTimer::singleShot( 150, this, SLOT( slotScrollToFirstCheckedItem() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTreeSelectionEditor::createEditorWidget( QWidget* parent )
{
    QFrame*      frame  = new QFrame( parent );
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    frame->setLayout( layout );

    {
        QHBoxLayout* headerLayout = new QHBoxLayout;
        headerLayout->setContentsMargins( 0, 0, 0, 0 );
        layout->addLayout( headerLayout );

        UiTreeSelectionEditorAttribute attrib;

        m_toggleAllCheckBox = new QCheckBox();
        headerLayout->addWidget( m_toggleAllCheckBox );

        connect( m_toggleAllCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( slotToggleAll() ) );

        m_textFilterLineEdit = new PlaceholderLineEdit( "Click to add filter" );
        // TODO: setPlaceholderText() was introduced in Qt 4.7
        // Use QLineEdit instead of PlaceholderLineEdit when Qt is upgraded
        // m_textFilterLineEdit->setPlaceholderText("Click to add filter");

        headerLayout->addWidget( m_textFilterLineEdit );

        connect( m_textFilterLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( slotTextFilterChanged() ) );
    }

    QTreeViewHeightHint* treeViewHeightHint = new QTreeViewHeightHint( parent );
    treeViewHeightHint->setHeightHint( 2000 );
    treeViewHeightHint->setHeaderHidden( true );

    m_treeView = treeViewHeightHint;

    m_treeView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_treeView, SIGNAL( customContextMenuRequested( QPoint ) ), SLOT( customMenuRequested( QPoint ) ) );

    layout->addWidget( treeViewHeightHint );

    return frame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTreeSelectionEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins UiTreeSelectionEditor::calculateLabelContentMargins() const
{
    QSize    editorSize     = m_textFilterLineEdit->sizeHint();
    QSize    labelSize      = m_label->sizeHint();
    int      heightDiff     = editorSize.height() - labelSize.height();
    QMargins contentMargins = m_label->contentsMargins();
    if ( heightDiff > 0 )
    {
        contentMargins.setTop( contentMargins.top() + heightDiff / 2 );
        contentMargins.setBottom( contentMargins.bottom() + heightDiff / 2 );
    }
    return contentMargins;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeSelectionEditor::isMultiRowEditor() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::customMenuRequested( const QPoint& pos )
{
    QMenu menu;

    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

    bool onlyHeadersInSelection = true;
    for ( const auto& mi : selectedIndexes )
    {
        QVariant v = m_proxyModel->data( mi, UiTreeSelectionQModel::headingRole() );
        if ( v.toBool() == false )
        {
            onlyHeadersInSelection = false;
        }
    }

    if ( onlyHeadersInSelection && selectedIndexes.size() > 0 )
    {
        {
            QAction* act = new QAction( "Sub Items On", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSubItemsOn() ) );

            menu.addAction( act );
        }

        {
            QAction* act = new QAction( "Sub Items Off", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSubItemsOff() ) );

            menu.addAction( act );
        }
    }
    else if ( selectedIndexes.size() > 0 )
    {
        {
            QAction* act = new QAction( "Set Selected On", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSelectedOn() ) );

            menu.addAction( act );
        }

        {
            QAction* act = new QAction( "Set Selected Off", this );
            connect( act, SIGNAL( triggered() ), SLOT( slotSetSelectedOff() ) );

            menu.addAction( act );
        }
    }

    if ( menu.actions().size() > 0 )
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_treeView->viewport()->mapToGlobal( pos );

        menu.exec( globalPos );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotSetSelectedOn()
{
    this->setCheckedStateOfSelected( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotSetSelectedOff()
{
    this->setCheckedStateOfSelected( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::setCheckedStateOfSelected( bool checked )
{
    if ( !m_proxyModel ) return;

    QItemSelection selectionInProxyModel  = m_treeView->selectionModel()->selection();
    QItemSelection selectionInSourceModel = m_proxyModel->mapSelectionToSource( selectionInProxyModel );
    m_model->setCheckedStateForItems( selectionInSourceModel.indexes(), checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotSetSubItemsOn()
{
    this->setCheckedStateForSubItemsOfSelected( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotSetSubItemsOff()
{
    this->setCheckedStateForSubItemsOfSelected( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::setCheckedStateForSubItemsOfSelected( bool checked )
{
    QModelIndexList selectedProxyIndexes = m_treeView->selectionModel()->selectedIndexes();
    QModelIndexList sourceIndexesToSubItems;

    for ( const auto& mi : selectedProxyIndexes )
    {
        for ( int i = 0; i < m_proxyModel->rowCount( mi ); i++ )
        {
            QModelIndex childProxyIndex = m_proxyModel->index( i, 0, mi );
            sourceIndexesToSubItems.push_back( m_proxyModel->mapToSource( childProxyIndex ) );
        }
    }

    m_model->setCheckedStateForItems( sourceIndexesToSubItems, checked );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotToggleAll()
{
    if ( m_toggleAllCheckBox->isChecked() )
    {
        checkAllItems();
    }
    else
    {
        unCheckAllItems();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotTextFilterChanged()
{
    QString searchString = m_textFilterLineEdit->text();
    searchString += "*";

    // Escape the characters '[' and ']' as these have special meaning for a search string
    // To be able to search for vector names in brackets, these must be escaped
    // See "Wildcard Matching" in Qt documentation
    searchString.replace( "[", "\\[" );
    searchString.replace( "]", "\\]" );

    QRegExp searcher( searchString, Qt::CaseInsensitive, QRegExp::WildcardUnix );
    m_proxyModel->setFilterRegExp( searcher );

    updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotClicked( const QModelIndex& index )
{
    QModelIndex lastUncheckedIndex = m_model->indexForLastUncheckedItem();
    m_model->clearIndexForLastUncheckedItem();

    QModelIndex proxyModelIndex = m_proxyModel->mapFromSource( lastUncheckedIndex );
    if ( proxyModelIndex == index )
    {
        // Early return to avoid changing the current item if an item was unchecked
        return;
    }

    if ( m_attributes.setCurrentIndexWhenItemIsChecked && index.isValid() )
    {
        QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
        if ( selectedIndexes.size() < 2 )
        {
            QVariant v = m_proxyModel->data( index, Qt::CheckStateRole );
            if ( v == Qt::Checked )
            {
                m_treeView->setCurrentIndex( index );
            }
        }
    }
    currentChanged( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::slotScrollToFirstCheckedItem()
{
    auto firstVisibleIndex = m_treeView->indexAt( m_treeView->viewport()->rect().topLeft() );
    auto lastVisibleIndex  = m_treeView->indexAt( m_treeView->viewport()->rect().bottomRight() );

    if ( !firstVisibleIndex.isValid() )
    {
        return;
    }

    if ( !lastVisibleIndex.isValid() )
    {
        return;
    }

    for ( int i = firstVisibleIndex.row(); i < lastVisibleIndex.row(); i++ )
    {
        auto treeViewIndex = m_proxyModel->index( i, 0 );

        if ( m_proxyModel->data( treeViewIndex, Qt::CheckStateRole ).toBool() )
        {
            // Do nothing if there is a checked and visible item in the view
            return;
        }
    }

    for ( int i = 0; i < m_proxyModel->rowCount(); i++ )
    {
        auto treeViewIndex = m_proxyModel->index( i, 0 );

        if ( m_proxyModel->data( treeViewIndex, Qt::CheckStateRole ).toBool() )
        {
            // Scroll to the first checked item if no checked items are visible
            m_treeView->scrollTo( treeViewIndex );

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::currentChanged( const QModelIndex& current )
{
    if ( m_attributes.singleSelectionMode )
    {
        m_proxyModel->setData( current, true, Qt::CheckStateRole );
    }

    if ( m_attributes.currentIndexFieldHandle )
    {
        FieldUiCapability* uiFieldHandle = m_attributes.currentIndexFieldHandle->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            QVariant v = m_proxyModel->data( current, UiTreeSelectionQModel::optionItemValueRole() );

            UiCommandSystemProxy::instance()->setUiValueToField( uiFieldHandle, v.value<Variant>() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::checkAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->setCheckedStateForItems( indices, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::unCheckAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->setCheckedStateForItems( indices, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndexList UiTreeSelectionEditor::allVisibleSourceModelIndices() const
{
    QModelIndexList indices;

    recursiveAppendVisibleSourceModelIndices( QModelIndex(), &indices );

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeSelectionEditor::recursiveAppendVisibleSourceModelIndices( const QModelIndex& parent,
                                                                      QModelIndexList*   sourceModelIndices ) const
{
    for ( int row = 0; row < m_proxyModel->rowCount( parent ); row++ )
    {
        QModelIndex mi = m_proxyModel->index( row, 0, parent );
        if ( mi.isValid() )
        {
            QVariant v = m_proxyModel->data( mi, UiTreeSelectionQModel::headingRole() );
            if ( v.toBool() == false )
            {
                sourceModelIndices->push_back( m_proxyModel->mapToSource( mi ) );
            }

            recursiveAppendVisibleSourceModelIndices( mi, sourceModelIndices );
        }
    }
}

} // end namespace caf
