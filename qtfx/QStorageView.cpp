#include "QStorageView.h"
#include "QAction"
#include "QHeaderView"
#include <algorithm>

using namespace QtCharts;

QStorageView::QStorageView( QStorageDataModel* m, QWidget* parent ) : QWidget( parent ), model( m ), currentUpdateIdx( 0 )
{
	itemList = new QTreeWidget( this );
	itemList->setRootIsDecorated( false );
	itemList->setColumnCount( 2 );
	QStringList headerLabels;
	headerLabels << "Variable" << "Value";
	itemList->setHeaderLabels( headerLabels );
	itemList->header()->setStretchLastSection( false );
	for ( int i = 0; i < itemList->columnCount(); ++i )
		itemList->header()->setSectionResizeMode( i, i == 0 ? QHeaderView::Stretch : QHeaderView::ResizeToContents );

	chart = new QChart();
	chartView = new QChartView( chart, this );
	splitter = new QSplitter( this );
	splitter->addWidget( itemList );
	splitter->addWidget( chartView );

	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setContentsMargins( 4, 4, 4, 4 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );
}

void QStorageView::refresh( double time, bool refreshAll )
{
	if ( itemList->topLevelItemCount() != model->getSize() )
		return reset();
	else if ( model->getSize() > 0 )
	{
		if ( isVisible() )
		{
			int itemCount = refreshAll ? model->getSize() : std::min<int>( smallRefreshItemCount, model->getSize() );
			for ( size_t i = 0; i < itemCount; ++i )
			{
				itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString().sprintf( "%.3f", model->getValue( currentUpdateIdx, time ) ) );
				++currentUpdateIdx %= model->getSize();
			}
		}
	}
}

void QStorageView::reset()
{
	itemList->clear();
	for ( size_t i = 0; i < model->getSize(); ++i )
	{
		auto* wdg = new QTreeWidgetItem( itemList, QStringList( model->getLabel( i ) ) );
		wdg->setTextAlignment( 1, Qt::AlignRight );
		wdg->setFlags( wdg->flags() | Qt::ItemIsUserCheckable );
		wdg->setCheckState( 0, Qt::Unchecked );
	}
	currentUpdateIdx = 0;
}
