#include "QFileBrowser.h"

QFileBrowser::QFileBrowser( QWidget* parent ) : QTreeView( parent )
{
	fileModel = new QFileSystemModel( this );
	setModel( fileModel );
	for ( int i = 1; i <= 3; ++i )
		hideColumn( i );
}

void QFileBrowser::setFolder( const QString& folder, const QString& filter )
{
	QDir().mkdir( folder );
	fileModel->setNameFilters( QStringList( "*.par" ) );
	setRootIndex( fileModel->setRootPath( folder ) );
}
