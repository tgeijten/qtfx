#include "QFileBrowser.h"

QFileBrowser::QFileBrowser( QWidget* parent, const QString& folder, const QString& filter ) : QTreeView( parent )
{
	fileModel = new QFileSystemModel( this );
	setModel( fileModel );
	for ( int i = 1; i <= 3; ++i )
		hideColumn( i );

	connect( this, &QTreeView::activated, this, &QFileBrowser::activateItem );
	connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QFileBrowser::selectItem );

	if ( !folder.isEmpty() )
		setRoot( folder, filter );
}

QFileBrowser::QFileBrowser( QWidget* parent, QFileSystemModel* model, const QString& folder, const QString& filter  )
{
	fileModel = model;
	model->setParent( this );
	setModel( model );
	for ( int i = 1; i <= 3; ++i )
		hideColumn( i );

	connect( this, &QTreeView::activated, this, &QFileBrowser::activateItem );
	connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QFileBrowser::selectItem );

	if ( !folder.isEmpty() )
		setRoot( folder, filter );
}

void QFileBrowser::setRoot( const QString& folder, const QString& filter )
{
	QDir().mkdir( folder );
	fileModel->setNameFilters( QStringList( filter ) );
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
