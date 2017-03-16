#include "QStorageView.h"
#include "QAction"

using namespace QtCharts;

QStorageView::QStorageView( QStorageDataModel* m, QWidget* parent ) : QWidget( parent ), model( m )
{
	tree = new QTreeWidget( this );
	tree->setColumnCount( 1 );
	chart = new QChart();
	chartView = new QChartView( chart, this );

	splitter = new QSplitter( this );
	splitter->addWidget( tree );
	splitter->addWidget( chartView );
}

void QStorageView::refresh()
{
	tree->clear();
	QList<QTreeWidgetItem *> items;
	for ( size_t i = 0; i < model->getSize(); ++i )
		items.append( new QTreeWidgetItem( QStringList( model->getLabel( i ) ) ) );
	tree->insertTopLevelItems( 0, items );
}
