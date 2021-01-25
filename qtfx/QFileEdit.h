#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

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

	void setText( const QString& s );
	void setText( const QString& s, Mode mode, const QString& filter = "" );
	QString text() const;
	void setFilter( const QString& filter );
	void setMode( Mode mode );

public slots:
	void browse();

private:
	QLineEdit* lineEdit;
	QAbstractButton* browseButton;
	QString filter;
	Mode mode;
};
