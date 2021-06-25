#include "QGroup.h"
#include "QBoxLayout"
#include "QFormLayout"
#include "QLabel"

QGroup::QGroup( QWidget* parent ) : QWidget( parent )
{}

void QGroup::add( QWidget* w )
{
	layout()->addWidget( w );
	w->setParent( this );
}

QGroup& QGroup::operator<<( QWidget* w )
{
	add( w );
	return *this;
}

void QGroup::clear()
{
	qDeleteAll( findChildren<QWidget*>( "", Qt::FindDirectChildrenOnly ) );
}

QVGroup::QVGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	QVBoxLayout* l = new QVBoxLayout;
	l->setMargin( margin );
	l->setSpacing( spacing );
	setLayout( l );
}

QHGroup::QHGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	QHBoxLayout* l = new QHBoxLayout;
	l->setMargin( margin );
	l->setSpacing( spacing );
	setLayout( l );
}

QFormGroup::QFormGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	QFormLayout* l = new QFormLayout;
	l->setMargin( margin );
	l->setSpacing( spacing );
	setLayout( l );
}

void QFormGroup::addRow( const QString& label, QWidget* w )
{
	auto fl = qobject_cast<QFormLayout*>( layout() );
	QLabel* l = new QLabel( label );
	l->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
	fl->addRow( l, w );
	w->setParent( this );
}
