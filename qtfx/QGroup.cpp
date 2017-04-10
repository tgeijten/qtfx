#include "QGroup.h"
#include "QBoxLayout"

QGroup::QGroup( QWidget* parent ) : QWidget( parent )
{
}

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
