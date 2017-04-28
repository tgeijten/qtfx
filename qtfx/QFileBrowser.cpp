#include "QFileBrowser.h"
#include "QTreeView"
#include "QHeaderView"

QFileBrowser::QFileBrowser( QWidget* parent, const QString& folder, const QString& filter, int num_columns, QFileSystemModel* model  ) :
QTreeView( parent ),
fileModel( nullptr )
{
	connect( this, &QTreeView::activated, this, &QFileBrowser::activateItem );
	connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QFileBrowser::selectItem );
	if ( !folder.isEmpty() )
		init( folder, filter, num_columns, model );
}

void QFileBrowser::init( const QString& folder, const QString& filter, int num_columns, QFileSystemModel* model )
{
	// attach model
	fileModel = model;
	if ( fileModel == nullptr )
		fileModel = new QFileSystemModel( this );
	else fileModel->setParent( this );
	setModel( fileModel );

	// set columns
	for ( int i = num_columns; i <= 3; ++i )
		hideColumn( i );

	header()->setStretchLastSection( false );
	for ( int i = 0; i < fileModel->columnCount(); ++i )
		header()->setSectionResizeMode( i, i == 0 ? QHeaderView::Stretch : QHeaderView::ResizeToContents );

	// set folder / filter
	QDir().mkdir( folder );
	fileModel->setNameFilters( QStringList( filter ) );
	fileModel->setNameFilterDisables( false );
	setRootIndex( fileModel->setRootPath( folder ) );
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
