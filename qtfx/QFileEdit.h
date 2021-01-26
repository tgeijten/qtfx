#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QAbstractButton>

class QFileEdit : public QWidget
{
	Q_OBJECT

public:
	enum Mode
	{
		OpenFile,
		SaveFile,
		Directory
	};

	QFileEdit( QWidget* parent, Mode mode = OpenFile, const QString& filter = "" );

	void init( Mode mode, const QString& filter, const QString& file, const QString& start_dir = "" );
	QString text() const;

public slots:
	void browse();
	void setText( const QString& s );

signals:
	void textChanged( const QString& );
	void editingFinished();
private:
	QLineEdit* lineEdit;
	QAbstractButton* browseButton;
	Mode mode;
	QString filter;
	QString startDir;
};
