#include "QFileBrowser.h"

QFileBrowser::QFileBrowser( QWidget* parent ) : QTreeView( parent )
{
	fileModel = new QFileSystemModel( this );
	setModel( fileModel );
	for ( int i = 1; i <= 3; ++i )
		hideColumn( i );

	connect( this, &QTreeView::activated, this, &QFileBrowser::activateItem );
	connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QFileBrowser::selectItem );
}

void QFileBrowser::setFolder( const QString& folder, const QString& filter )
{
	QDir().mkdir( folder );
	fileModel->setNameFilters( QStringList( filter ) );
	setRootIndex( fileModel->setRootPath( folder ) );
}

void QFileBrowser::activateItem( const QModelIndex& idx )
{
	emit activated( fileModel->fileInfo( idx ) );
}

void QFileBrowser::selectItem( const QModelIndex& a, const QModelIndex& b )
{
	emit selectionChanged( fileModel->fileInfo( a ), fileModel->fileInfo( b ) );
}
