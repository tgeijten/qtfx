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

QVGroup::QVGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	layout_ = new QVBoxLayout;
	layout_->setMargin( margin );
	layout_->setSpacing( spacing );
	setLayout( layout_ );
}

QHGroup::QHGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	layout_ = new QHBoxLayout;
	layout_->setMargin( margin );
	layout_->setSpacing( spacing );
	setLayout( layout_ );
}

QFormGroup::QFormGroup( QWidget* parent, int margin, int spacing ) :
	QGroup( parent )
{
	layout_ = new QFormLayout;
	layout_->setMargin( margin );
	layout_->setSpacing( spacing );
	setLayout( layout_ );
}

void QFormGroup::addRow( const QString& label, QWidget* w )
{
	QLabel* l = new QLabel( label );
	l->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
	layout_->addRow( l, w );
	w->setParent( this );
}
