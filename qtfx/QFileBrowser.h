#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>

class QFileBrowser : public QTreeView
{
	Q_OBJECT

public:
	QFileBrowser( QWidget* parent, const QString& folder = "", const QString& filter = "*.*" );
	QFileBrowser( QWidget* parent, QFileSystemModel* model, const QString& folder = "", const QString& filter = "*.*" );

	void setRoot( const QString& folder, const QString& filter = "*.*" );
	virtual ~QFileBrowser() {}

	QFileSystemModel* fileSystemModel() { return fileModel; }

signals:
	void itemTriggered( QString filename );
	void selectionChanged( QString current, QString previous );

private slots:
	void activateItem( const QModelIndex& idx );
	void selectItem( const QModelIndex& a, const QModelIndex& b );

protected:
	QFileSystemModel* fileModel;
};
