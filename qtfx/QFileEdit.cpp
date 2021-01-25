#include "QFileEdit.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QToolButton>

QFileEdit::QFileEdit( QWidget* parent, Mode m, const QString& f ) :
	QWidget( parent ),
	mode( m ),
	filter( f )
{
	auto* l = new QHBoxLayout( this );
	l->setSpacing( 2 );
	l->setMargin( 0 );
	setLayout( l );

	lineEdit = new QLineEdit( this );
	l->addWidget( lineEdit );

	browseButton = new QToolButton( this );
	browseButton->setText( "..." );
	browseButton->setContentsMargins( 0, 0, 0, 0 );
	l->addWidget( browseButton );

	connect( browseButton, &QPushButton::released, this, &QFileEdit::browse );
}

void QFileEdit::browse()
{
	QFileInfo fi = lineEdit->text();

	QString result;

	switch ( mode )
	{
	case QFileEdit::OpenFile:
		result = QFileDialog::getOpenFileName( this, "Open File", fi.path(), filter, nullptr );
		break;
	case QFileEdit::SaveFile:
		result = QFileDialog::getSaveFileName( this, "Save File", fi.path(), filter, nullptr );
		break;
	case QFileEdit::Directory:
		result = QFileDialog::getExistingDirectory( this, "Select Directory", fi.path(), nullptr );
		break;
	default:
		break;
	}

	if ( !result.isEmpty() )
		lineEdit->setText( result );
}

void QFileEdit::setText( const QString& s )
{
	lineEdit->setText( s );
}

void QFileEdit::setText( const QString& s, Mode m, const QString& f )
{
	setText( s );
	setMode( m );
	setFilter( f );
}

QString QFileEdit::text() const
{
	return lineEdit->text();
}

void QFileEdit::setFilter( const QString& f )
{
	filter = f;
}

void QFileEdit::setMode( Mode m )
{
	mode = m;
}
