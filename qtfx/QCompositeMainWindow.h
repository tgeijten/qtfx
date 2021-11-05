#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QStatusBar>
#include <QSettings>
#include <QMenu>

class QCompositeMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	QCompositeMainWindow( QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
	virtual ~QCompositeMainWindow();

	virtual void openFile( const QString& filename ) { }
	virtual void saveFile() { }
	virtual void saveFileAs( const QString& filename ) { }
	virtual bool tryExit() { return true; }

	QDockWidget* createDockWidget( const QString& title, QWidget* widget, Qt::DockWidgetArea area );
	int registerDockWidget( QDockWidget* widget, const QString& menu_text );

public slots:
	virtual void fileOpenTriggered();
	virtual void fileOpenRecentTriggered();
	virtual void fileCloseTriggered();
	virtual void fileSaveTriggered();
	virtual void fileSaveAsTriggered();
	virtual void fileExitTriggered();
	virtual void windowMenuTriggered();

	void updateRecentFilesMenu( const QString& filename = "" );

protected:
	template< typename T1, typename T2 >
	QAction* addMenuAction( QMenu* menu, const QString& text, T1* funcObj, T2 func, const QKeySequence& shortcut = QKeySequence(), bool separator = false ) {
		QAction* a = menu->addAction( text );
		a->setShortcut( shortcut );
		connect( a, &QAction::triggered, funcObj, func );
		if ( separator ) menu->addSeparator();
		return a;
	}

	void setActiveFile( const QString& filename ) { activeFile = filename; }

	QStatusBar* createStatusBar();

	QMenu* createFileMenu( const QString& default_folder, const QString& file_types );
	QMenu* createWindowMenu();

	virtual void restoreCustomSettings( QSettings& settings ) {}
	virtual void saveCustomSettings( QSettings& settings ) {}

	void createSettings( const QString& company, const QString& app );
	void restoreSettings( bool skipGeometry = false );
	void saveSettings();

	virtual void closeEvent( QCloseEvent *event ) override;

	void information( const QString& title, const QString& message );
	void warning( const QString& title, const QString& message );
	bool question( const QString& title, const QString& message );
	void error( const QString& title, const QString& message );

	QString fileFolder;
	QString fileTypes;
	QString activeFile;
	QAction* recentFilesMenu;
	QMenu* windowMenu;
	QStatusBar* statusBar;
	std::vector< QDockWidget* > dockWidgets;
	QStringList recentFiles;
	QSettings* settings;
};
