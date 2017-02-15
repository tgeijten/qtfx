#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QStatusBar>

class QCompositeMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	QCompositeMainWindow( QWidget* parent = nullptr );
	virtual ~QCompositeMainWindow();

	virtual void fileOpen( const QString& filename ) {}
	virtual void fileSaveAs( const QString& filename ) {}

public slots:
	virtual void fileOpen();
	virtual void fileOpenRecent();
	virtual void fileClose();
	virtual void fileSave();
	virtual void fileSaveAs();
	virtual void fileExit();

	void updateViewMenu();

protected:
	void createFileMenu( const QString& name = "File" );
	void createViewMenu();
	void createHelpMenu();
	QDockWidget* createDockWidget( QWidget* parent, const QString& title );

	void addRecentFile( const QString& file );

	void restoreSettings();
	void saveSettings();

	QStringList recentFiles;
	QMenuBar* menuBar;
	QMenu* fileMenu;
	QMenu* viewMenu;
	QMenu* helpMenu;
	QStatusBar* statusBar;
	std::vector< QDockWidget* > dockWidgets;
};
