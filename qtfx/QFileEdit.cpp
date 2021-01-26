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

	connect( browseButton, &QAbstractButton::released, this, &QFileEdit::browse );
	connect( lineEdit, &QLineEdit::textChanged, this, &QFileEdit::textChanged );
	connect( lineEdit, &QLineEdit::editingFinished, this, &QFileEdit::editingFinished );
}

void QFileEdit::init( Mode m, const QString& f, const QString& file, const QString& start_dir )
{
	mode = m;
	filter = f;
	setText( file );
	startDir = start_dir;
}

void QFileEdit::browse()
{
	QString current = text();
	QString result;

	// update startdir if not set
	if ( startDir.isEmpty() && !current.isEmpty())
	{
		if ( mode == Directory )
			startDir = current;
		else startDir = QFileInfo( current ).path();
	}

	switch ( mode )
	{
	case QFileEdit::OpenFile:
		result = QFileDialog::getOpenFileName( this, "Open File", startDir, filter, nullptr );
		break;
	case QFileEdit::SaveFile:
		result = QFileDialog::getSaveFileName( this, "Save File", startDir, filter, nullptr );
		break;
	case QFileEdit::Directory:
		result = QFileDialog::getExistingDirectory( this, "Select Directory", startDir, nullptr );
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

QString QFileEdit::text() const
{
	return lineEdit->text();
}
