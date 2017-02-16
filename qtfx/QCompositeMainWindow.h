#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QStatusBar>

class QCompositeMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	QCompositeMainWindow( QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
	virtual ~QCompositeMainWindow();

	virtual void openFile( const QString& filename ) { }
	virtual void saveFile() { }
	virtual void saveFileAs( const QString& filename ) { }

	QDockWidget* createDockWidget( const QString& title, QWidget* widget, Qt::DockWidgetArea area );

public slots:
	virtual void fileOpenTriggered();
	virtual void fileOpenRecentTriggered();
	virtual void fileCloseTriggered();
	virtual void fileSaveTriggered();
	virtual void fileSaveAsTriggered();
	virtual void fileExitTriggered();
	virtual void viewMenuTriggered();

	void updateViewMenu();
	void updateRecentFilesMenu( const QString& filename = "" );

protected:
	template< typename T >
	QAction* addMenuAction( QMenu* menu, const QString& text, const QString& shortcut, QObject* funcObj, T func, bool separator = false ) {
		QAction* a = menu->addAction( text );
		a->setShortcut( shortcut );
		connect( a, &QAction::triggered, this, func );
		if ( separator ) menu->addSeparator();
		return a;
	}

	void setActiveFile( const QString& filename ) { activeFile = filename; }

	QMenuBar* acquireMenuBar();
	QStatusBar* createStatusBar();
	void createFileMenu( const QString& default_folder, const QString& file_types );
	QString fileFolder;
	QString fileTypes;
	QString activeFile;

	void createViewMenu();
	void createHelpMenu();

	void restoreSettings();
	void saveSettings();

	QWidget* centralWidget;
	QMenuBar* menuBar;
	QMenu* fileMenu;
	QAction* recentFilesMenu;
	QMenu* viewMenu;
	QMenu* helpMenu;
	QStatusBar* statusBar;
	std::vector< QDockWidget* > dockWidgets;
	QStringList recentFiles;
};
