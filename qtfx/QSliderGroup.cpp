#include "QSliderGroup.h"

#include "QFormLayout"

QSliderGroup::QSliderGroup( QWidget* parent, int margin, int spacing ) :
	QWidget( parent )
{
	auto lo = new QFormLayout( this );
	lo->setMargin( margin );
	lo->setSpacing( spacing );
	lo->setLabelAlignment( Qt::AlignVCenter );
}

QValueSlider* QSliderGroup::addSlider( const QString& name )
{
	auto lo = qobject_cast<QFormLayout*>( layout() );
	auto s = new QValueSlider( this, 0, 4 );
	auto label = new QLabel( name, this );
	label->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
	lo->addRow( label, s );
	sliders.push_back( s );

	return s;
}

void QSliderGroup::clear()
{
	qDeleteAll( findChildren<QWidget*>( "", Qt::FindDirectChildrenOnly ) );
	sliders.clear();
}
