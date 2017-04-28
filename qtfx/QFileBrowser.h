#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>

class QFileBrowser : public QTreeView
{
	Q_OBJECT

public:
	QFileBrowser( QWidget* parent, const QString& folder = "", const QString& filter = "*.*", int num_columns = 1, QFileSystemModel* model = nullptr );

	void init( const QString& folder, const QString& filter = "*.*", int num_columns = 1, QFileSystemModel* model = nullptr );
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
	virtual void resizeEvent( QResizeEvent *event ) override;
};
