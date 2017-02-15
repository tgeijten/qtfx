#include "QCompositeMainWindow.h"
#include <QMainWindow>
#include <QDockWidget>
#include "QBoxLayout"

QCompositeMainWindow::QCompositeMainWindow( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags )
{

}

QCompositeMainWindow::~QCompositeMainWindow()
{}

void QCompositeMainWindow::fileOpen()
{

}

void QCompositeMainWindow::fileOpenRecent()
{

}

void QCompositeMainWindow::fileClose()
{

}

void QCompositeMainWindow::fileSave()
{

}

void QCompositeMainWindow::fileSaveAs()
{

}

void QCompositeMainWindow::fileExit()
{

}

void QCompositeMainWindow::updateViewMenu()
{

}

void QCompositeMainWindow::createFileMenu( const QString& name )
{

}

void QCompositeMainWindow::createViewMenu()
{

}

void QCompositeMainWindow::createHelpMenu()
{

}

QDockWidget* QCompositeMainWindow::createDockWidget( const QString& title, QWidget* widget, Qt::DockWidgetArea area  )
{
	QWidget* layoutWidget = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout( layoutWidget );
	layout->setSpacing( 1 );
	layout->setContentsMargins( 1, 1, 1, 1 );
	//layout->setObjectName( QStringLiteral( "verticalLayout" ) );
	layout->addWidget( widget );

	QDockWidget* d = new QDockWidget( title, this );
	d->setWidget( layoutWidget );

	addDockWidget( area, d );


	return nullptr;
}

void QCompositeMainWindow::addRecentFile( const QString& file )
{

}

void QCompositeMainWindow::restoreSettings()
{

}

void QCompositeMainWindow::saveSettings()
{

}
