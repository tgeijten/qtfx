#include "qtfx.h"

#include "QBoxLayout"
#include "QLabel"

QWidget* createVBoxWidget( QWidget* parent, int margin, int spacing )
{
	QWidget* w = new QWidget( parent );
	QVBoxLayout* l = new QVBoxLayout;
	l->setContentsMargins( margin, margin, margin, margin );
	l->setSpacing( spacing );
	l->addWidget( w );
	return w;
}

QWidget* createHBoxWidget( QWidget* parent, int margin, int spacing )
{
	QWidget* w = new QWidget( parent );
	QHBoxLayout* l = new QHBoxLayout;
	l->setContentsMargins( margin, margin, margin, margin );
	l->setSpacing( spacing );
	l->addWidget( w );
	return w;
}
