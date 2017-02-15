#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>

class QFileBrowser : public QTreeView
{
	Q_OBJECT

public:
	QFileBrowser( QWidget* parent = 0 );

	void setFolder( const QString& folder, const QString& filter = "*.*" );
	virtual ~QFileBrowser() {}

	QFileSystemModel* fileSystemModel() { return fileModel; }

public slots:

signals:
	void activated( QFileInfo filename );
	void selectionChanged( QFileInfo current, QFileInfo previous );

private slots:
	void activateItem( const QModelIndex& idx );
	void selectItem( const QModelIndex& a, const QModelIndex& b );


protected:
	QFileSystemModel* fileModel;
};
