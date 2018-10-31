#include "QFileBrowser.h"
#include "QTreeView"
#include "QHeaderView"

QFileBrowser::QFileBrowser( QWidget* parent, const QString& folder, const QString& filter, int num_columns, QFileSystemModel* model  ) :
QTreeView( parent ),
fileModel( nullptr )
{
	setModel( model );
	setRoot( folder, filter );
	setNumColumns( num_columns );

	connect( this, &QTreeView::activated, this, &QFileBrowser::activateItem );
	connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QFileBrowser::selectItem );
}

void QFileBrowser::setModel( QFileSystemModel* model )
{
	// detach existing (if any) -- perhaps this is not needed
	if ( fileModel )
		fileModel->setParent( nullptr );

	// attach model
	fileModel = model;
	if ( fileModel == nullptr )
		fileModel = new QFileSystemModel( this );
	else fileModel->setParent( this );
	QTreeView::setModel( fileModel );
}

void QFileBrowser::setRoot( const QString& folder, const QString& filter )
{
	if ( fileModel && !folder.isEmpty() )
	{
		// set folder / filter
		QDir().mkdir( folder );
		fileModel->setNameFilters( filter.split( ';' ) );
		fileModel->setNameFilterDisables( false );
		setRootIndex( fileModel->setRootPath( folder ) );
	}
}

void QFileBrowser::setNumColumns( int num_columns )
{
	// set columns
	for ( int i = num_columns; i <= 3; ++i )
		hideColumn( i );

	header()->setStretchLastSection( false );
	for ( int i = 0; i < fileModel->columnCount(); ++i )
		header()->setSectionResizeMode( i, i == 0 ? QHeaderView::Stretch : QHeaderView::ResizeToContents );
}

void QFileBrowser::activateItem( const QModelIndex& idx )
{
	emit itemTriggered( fileModel->fileInfo( idx ).absoluteFilePath() );
}

void QFileBrowser::selectItem( const QModelIndex& a, const QModelIndex& b )
{
	emit selectionChanged( fileModel->fileInfo( a ).absoluteFilePath(), fileModel->fileInfo( b ).absoluteFilePath() );
}

void QFileBrowser::resizeEvent( QResizeEvent *event )
{
	QTreeView::resizeEvent( event );
}
