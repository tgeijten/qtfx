#include "QCompositeMainWindow.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QBoxLayout>
#include <QMenuBar>
#include <iostream>
#include <QFileDialog>
#include <QSettings>
#include "qevent.h"

using std::cout;
using std::endl;

QCompositeMainWindow::QCompositeMainWindow( QWidget* parent, Qt::WindowFlags flags ) :
QMainWindow( parent, flags ),
menuBar( nullptr ),
fileMenu( nullptr ),
viewMenu( nullptr ),
settings( nullptr )
{
	centralWidget = new QWidget( this );
	QLayout* centralLayout = new QVBoxLayout( centralWidget );
	centralLayout->setSpacing( 0 );
	centralLayout->setContentsMargins( 0, 0, 0, 0 );
	setCentralWidget( centralWidget );
}

QCompositeMainWindow::~QCompositeMainWindow()
{
	if ( settings )
		delete settings;
}

void QCompositeMainWindow::fileOpenTriggered()
{
	QString filename = QFileDialog::getOpenFileName( this, "Open File", fileFolder, fileTypes );
	if ( !filename.isEmpty() )
	{
		openFile( filename );
		updateRecentFilesMenu( filename );
	}
}

void QCompositeMainWindow::fileOpenRecentTriggered()
{
	QString filename = qobject_cast<QAction*>( sender() )->text();
	openFile( filename );
	updateRecentFilesMenu( filename );
}

void QCompositeMainWindow::fileCloseTriggered()
{
}

void QCompositeMainWindow::fileSaveTriggered()
{
	updateRecentFilesMenu( activeFile );
	saveFile();
}

void QCompositeMainWindow::fileSaveAsTriggered()
{
	QString filename = QFileDialog::getSaveFileName( this, "Save File", fileFolder, fileTypes );
	if ( !filename.isEmpty() )
	{
		saveFileAs( filename );
		updateRecentFilesMenu( filename );
	}
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

void QCompositeMainWindow::updateRecentFilesMenu( const QString& filename )
{
	// add to list (if any)
	if ( !filename.isEmpty() )
	{
		recentFiles.push_front( filename );
		recentFiles.removeDuplicates();
		while ( recentFiles.size() > 10 )
			recentFiles.removeLast();
	}

	// init recent files menu
	QMenu* recent_menu = new QMenu();
	for ( int idx = 0; idx < recentFiles.size(); ++idx )
	{
		QAction* act = recent_menu->addAction( recentFiles[ idx ] );
		connect( act, SIGNAL( triggered() ), this, SLOT( fileOpenRecentTriggered() ) );
	}
	recentFilesMenu->setMenu( recent_menu );
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

void QCompositeMainWindow::createFileMenu( const QString& default_folder, const QString& file_types )
{
	fileMenu = acquireMenuBar()->addMenu( ( "&File" ) );

	addMenuAction( fileMenu, "&Open...", this, &QCompositeMainWindow::fileOpenTriggered, QKeySequence( "Ctrl+O" ) );
	recentFilesMenu = addMenuAction( fileMenu, "Open &Recent", this, &QCompositeMainWindow::fileOpenTriggered, QKeySequence(), true );
	addMenuAction( fileMenu, "&Save", this, &QCompositeMainWindow::fileSaveTriggered, QKeySequence( "Ctrl+S" ) );
	addMenuAction( fileMenu, "Save &As...", this, &QCompositeMainWindow::fileSaveAsTriggered, QKeySequence( "Ctrl+Shift+S" ) );
	addMenuAction( fileMenu, "&Close", this, &QCompositeMainWindow::fileCloseTriggered, QKeySequence( "Ctrl+F4" ), true );
	addMenuAction( fileMenu, "E&xit", this, &QCompositeMainWindow::fileCloseTriggered, QKeySequence( "Alt+X" ) );

	fileFolder = default_folder;
	fileTypes = file_types;
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
	d->setObjectName( title );
	d->setWidget( layoutWidget );

	addDockWidget( area, d );

	// add to view menu
	if ( viewMenu )
		addMenuAction( viewMenu, title, this, &QCompositeMainWindow::viewMenuTriggered )->setCheckable( true );

	return nullptr;
}

void QCompositeMainWindow::restoreSettings( const QString& company, const QString& app )
{
	if ( settings )
		delete settings;

	settings = new QSettings( company, app );

	restoreGeometry( settings->value( "geometry" ).toByteArray() );
	restoreState( settings->value( "windowState" ).toByteArray() );
	recentFiles = settings->value( "recentFiles" ).toStringList();
	updateRecentFilesMenu();
	updateViewMenu();

	restoreCurstomSettings( *settings );
}

void QCompositeMainWindow::saveSettings()
{
	settings->setValue( "geometry", saveGeometry() );
	settings->setValue( "windowState", saveState() );
	settings->setValue( "recentFiles", recentFiles );
	saveCustomSettings( *settings );
}

void QCompositeMainWindow::closeEvent( QCloseEvent *event )
{
	if ( canClose() )
	{
		if ( settings )
			saveSettings();

		event->accept();
	}
	else event->ignore();
}
