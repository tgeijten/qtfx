#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QStatusBar>
#include <QSettings>

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
	template< typename T1, typename T2 >
	QAction* addMenuAction( QMenu* menu, const QString& text, const QString& shortcut, T1* funcObj, T2 func, bool separator = false ) {
		QAction* a = menu->addAction( text );
		a->setShortcut( shortcut );
		connect( a, &QAction::triggered, funcObj, func );
		if ( separator ) menu->addSeparator();
		return a;
	}

	virtual bool canClose() { return true; }

	void setActiveFile( const QString& filename ) { activeFile = filename; }

	QMenuBar* acquireMenuBar();
	QStatusBar* createStatusBar();
	void createFileMenu( const QString& default_folder, const QString& file_types );
	QString fileFolder;
	QString fileTypes;
	QString activeFile;

	void createViewMenu();
	void createHelpMenu();

	void useSettings( const QString& company, const QString& app );
	void saveSettings();

	virtual void closeEvent( QCloseEvent *event ) override;

	QWidget* centralWidget;
	QMenuBar* menuBar;
	QMenu* fileMenu;
	QAction* recentFilesMenu;
	QMenu* viewMenu;
	QMenu* helpMenu;
	QStatusBar* statusBar;
	std::vector< QDockWidget* > dockWidgets;
	QStringList recentFiles;
	QSettings* settings;
};
