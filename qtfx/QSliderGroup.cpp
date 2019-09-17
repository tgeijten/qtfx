#include "QSliderGroup.h"

#include "QFormLayout"

QSliderGroup::QSliderGroup( QWidget* parent, int margin, int spacing ) :
	QWidget( parent )
{
	auto l = new QFormLayout( this );
	l->setMargin( margin );
	l->setSpacing( spacing );
}

QSlider* QSliderGroup::addSlider( const QString& name )
{
	auto s = new QSlider( Qt::Horizontal, this );
	s->setRange( -100, 100 );
	qobject_cast<QFormLayout*>( layout() )->addRow( name, s );

	sliders.push_back( s );

	return s;
}

void QSliderGroup::clear()
{
	qDeleteAll( findChildren<QWidget*>( "", Qt::FindDirectChildrenOnly ) );
	sliders.clear();
}
