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

protected:
	QFileSystemModel* fileModel;
};
