#include "QCompositeMainWindow.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>
#include <QSettings>
#include <qevent.h>
#include <QMimeData>

using std::cout;
using std::endl;

QCompositeMainWindow::QCompositeMainWindow( QWidget* parent, Qt::WindowFlags flags ) :
	QMainWindow( parent, flags ),
	recentFilesMenu( nullptr ),
	windowMenu( nullptr ),
	statusBar( nullptr ),
	settings( nullptr )
{
	setAcceptDrops( true );
}

QCompositeMainWindow::~QCompositeMainWindow()
{}

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
	close();
}

void QCompositeMainWindow::windowMenuTriggered()
{
	QAction* menu_item = qobject_cast<QAction*>( sender() );
	int index = menu_item->data().toInt();

	if ( index >= 0 && index < dockWidgets.size() )
	{
		dockWidgets[index].first->show();
		dockWidgets[index].first->raise();
		if ( dockWidgets[index].second )
			dockWidgets[index].second->setFocus();
	}
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
		QAction* act = recent_menu->addAction( recentFiles[idx] );
		connect( act, SIGNAL( triggered() ), this, SLOT( fileOpenRecentTriggered() ) );
	}
	if ( recentFilesMenu )
		recentFilesMenu->setMenu( recent_menu );
}

QStatusBar* QCompositeMainWindow::createStatusBar()
{
	statusBar = new QStatusBar( this );
	setStatusBar( statusBar );
	return statusBar;
}

QMenu* QCompositeMainWindow::createFileMenu( const QString& default_folder, const QString& file_types )
{
	auto fileMenu = menuBar()->addMenu( ( "&File" ) );

	addMenuAction( fileMenu, "&Open...", this, &QCompositeMainWindow::fileOpenTriggered, QKeySequence( "Ctrl+O" ) );
	recentFilesMenu = addMenuAction( fileMenu, "Open &Recent", this, &QCompositeMainWindow::fileOpenTriggered, QKeySequence() );
	fileMenu->addSeparator();
	addMenuAction( fileMenu, "&Save", this, &QCompositeMainWindow::fileSaveTriggered, QKeySequence( "Ctrl+S" ) );
	addMenuAction( fileMenu, "Save &As...", this, &QCompositeMainWindow::fileSaveAsTriggered, QKeySequence( "Ctrl+Shift+S" ) );
	addMenuAction( fileMenu, "&Close", this, &QCompositeMainWindow::fileCloseTriggered, QKeySequence( "Ctrl+W" ) );
	fileMenu->addSeparator();
	addMenuAction( fileMenu, "E&xit", this, &QCompositeMainWindow::fileExitTriggered, QKeySequence( "Alt+X" ) );

	fileFolder = default_folder;
	fileTypes = file_types;

	return fileMenu;
}

QMenu* QCompositeMainWindow::createWindowMenu()
{
	windowMenu = menuBar()->addMenu( ( "&Window" ) );
	return windowMenu;
}

QDockWidget* QCompositeMainWindow::createDockWidget( const QString& title, QWidget* widget, Qt::DockWidgetArea area, QWidget* focusWidget, const QKeySequence& shortcut )
{
	QWidget* layoutWidget = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout( layoutWidget );
	layout->setSpacing( 10 );
	layout->setContentsMargins( 1, 1, 1, 1 );
	layout->addWidget( widget );

	QString cleanTitle = QString( title ).replace( "&", "" );

	QDockWidget* d = new QDockWidget( cleanTitle, this );
	d->setObjectName( cleanTitle );
	d->setWidget( layoutWidget );

	addDockWidget( area, d );
	registerDockWidget( d, title, focusWidget, shortcut );

	return d;
}

int QCompositeMainWindow::registerDockWidget( QDockWidget* widget, const QString& menu_text, QWidget* focusWidget, const QKeySequence& shortcut )
{
	dockWidgets.push_back( { widget, focusWidget } );
	int index = static_cast<int>( dockWidgets.size() ) - 1;
	if ( windowMenu )
	{
		QAction* a = windowMenu->addAction( menu_text, this, &QCompositeMainWindow::windowMenuTriggered, shortcut );
		a->setData( index );
	}
	return index;
}

void QCompositeMainWindow::createSettings( const QString& company, const QString& app )
{
	if ( settings )
		delete settings;
	settings = new QSettings( company, app );
}

void QCompositeMainWindow::restoreSettings( bool skipGeometry )
{
	if ( settings )
	{
		if ( !skipGeometry )
		{
			restoreGeometry( settings->value( "geometry" ).toByteArray() );
			restoreState( settings->value( "windowState" ).toByteArray() );
		}
		recentFiles = settings->value( "recentFiles" ).toStringList();
		updateRecentFilesMenu();
		restoreCustomSettings( *settings );
	}
}

void QCompositeMainWindow::saveSettings()
{
	if ( settings )
	{
		settings->setValue( "geometry", saveGeometry() );
		settings->setValue( "windowState", saveState() );
		settings->setValue( "recentFiles", recentFiles );
		saveCustomSettings( *settings );
	}
}

void QCompositeMainWindow::closeEvent( QCloseEvent* event )
{
	if ( tryExit() )
	{
		if ( settings )
			saveSettings();
		event->accept();
	}
	else event->ignore();
}

void QCompositeMainWindow::dragEnterEvent( QDragEnterEvent* event )
{
	if ( event->mimeData()->hasUrls() ) {
		event->acceptProposedAction();
	}
}

void QCompositeMainWindow::dropEvent( QDropEvent* event )
{
	for ( const QUrl& url : event->mimeData()->urls() )
	{
		QString fileName = url.toLocalFile();
		openFile( fileName );
	}
}

void QCompositeMainWindow::information( const QString& title, const QString& message )
{
	QMessageBox::information( this, title, message );
}

void QCompositeMainWindow::warning( const QString& title, const QString& message )
{
	QMessageBox::warning( this, title, message );
}

bool QCompositeMainWindow::question( const QString& title, const QString& message )
{
	return QMessageBox::question( this, title, message ) == QMessageBox::Yes;
}

void QCompositeMainWindow::error( const QString& title, const QString& message )
{
	QMessageBox::critical( this, title, message );
}
