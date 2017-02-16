#include "QCompositeMainWindow.h"
#include <QMainWindow>
#include <QDockWidget>
#include "QBoxLayout"
#include "QMenuBar"
#include <iostream>

using std::cout;
using std::endl;

QCompositeMainWindow::QCompositeMainWindow( QWidget* parent, Qt::WindowFlags flags ) :
QMainWindow( parent, flags ),
menuBar( nullptr ),
fileMenu( nullptr ),
viewMenu( nullptr )
{
	centralWidget = new QWidget( this );
	QLayout* centralLayout = new QVBoxLayout( centralWidget );
	centralLayout->setSpacing( 10 );
	centralLayout->setContentsMargins( 10, 10, 10, 10 );
	setCentralWidget( centralWidget );
}

QCompositeMainWindow::~QCompositeMainWindow()
{}

void QCompositeMainWindow::fileOpenTriggered()
{
}

void QCompositeMainWindow::fileOpenRecentTriggered()
{

}

void QCompositeMainWindow::fileCloseTriggered()
{

}

void QCompositeMainWindow::fileSaveTriggered()
{

}

void QCompositeMainWindow::fileSaveAsTriggered()
{

}

void QCompositeMainWindow::fileExitTriggered()
{

}

void QCompositeMainWindow::viewMenuTriggered()
{
	cout << qobject_cast<QAction*>( sender() )->text().toStdString();
}

void QCompositeMainWindow::updateViewMenu()
{

}

QMenuBar* QCompositeMainWindow::acquireMenuBar()
{
	if ( !menuBar )
	{
		menuBar = new QMenuBar( this );
		setMenuBar( menuBar );
	}
	return menuBar;
}

QStatusBar* QCompositeMainWindow::createStatusBar()
{
	statusBar = new QStatusBar( this );
	setStatusBar( statusBar );
	return statusBar;
}

void QCompositeMainWindow::createFileMenu( const QString& name )
{
	fileMenu = acquireMenuBar()->addMenu( ( "&File" ) );

	addMenuAction( fileMenu, "&Open", "Ctrl+O", this, &QCompositeMainWindow::fileOpenTriggered );
	addMenuAction( fileMenu, "&Close", "Ctrl+F4", this, &QCompositeMainWindow::fileCloseTriggered, true );
	addMenuAction( fileMenu, "E&xit", "Alt+X", this, &QCompositeMainWindow::fileCloseTriggered );
}

void QCompositeMainWindow::createViewMenu()
{
	viewMenu = acquireMenuBar()->addMenu( ( "&View" ) );
}

void QCompositeMainWindow::createHelpMenu()
{

}

QDockWidget* QCompositeMainWindow::createDockWidget( const QString& title, QWidget* widget, Qt::DockWidgetArea area  )
{
	QWidget* layoutWidget = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout( layoutWidget );
	layout->setSpacing( 10 );
	layout->setContentsMargins( 1, 1, 1, 1 );
	layout->addWidget( widget );

	QDockWidget* d = new QDockWidget( title, this );
	d->setWidget( layoutWidget );

	addDockWidget( area, d );

	// add to view menu
	if ( viewMenu )
		addMenuAction( viewMenu, title, "", this, &QCompositeMainWindow::viewMenuTriggered )->setCheckable( true );


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
