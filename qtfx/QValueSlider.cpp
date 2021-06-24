#include "QValueSlider.h"

#include "QBoxLayout"
#include "xo/numerical/math.h"

QValueSlider::QValueSlider( QWidget* parent, double stepSize, int margin, int spacing ) :
	QWidget( parent ),
	stepSize_( stepSize )
{
	QHBoxLayout* l = new QHBoxLayout;
	l->setMargin( margin );
	l->setSpacing( spacing );
	setLayout( l );

	spin_ = new QDoubleSpinBox( this );
	spin_->setSingleStep( stepSize_ );
	spin_->setDecimals( xo::round_cast<int>( log10( 1 / stepSize_ ) ) );
	connect( spin_, SIGNAL( valueChanged( double ) ), this, SLOT( spinValueChanged( double ) ) );
	l->addWidget( spin_ );

	min_ = new QLabel( this );
	min_->setDisabled( true );
	l->addWidget( min_ );

	slider_ = new QSlider( Qt::Horizontal, this );
	slider_->setSingleStep( 1 );
	slider_->setPageStep( 10 );
	slider_->setTickInterval( 10 );
	connect( slider_, SIGNAL( actionTriggered( int ) ), this, SLOT( sliderAction( int ) ) );
	l->addWidget( slider_ );

	max_ = new QLabel( this );
	max_->setDisabled( true );
	l->addWidget( max_ );
}

void QValueSlider::spinValueChanged( double d )
{
	slider_->setValue( to_int( d ) );
	emit valueChanged( d );
}

void QValueSlider::setRange( double min, double max )
{
	spin_->setRange( min, max );
	slider_->setRange( to_int( min ), to_int( max ) );
	min_->setText( QString::asprintf( "%g", min ) );
	max_->setText( QString::asprintf( "%g", max ) );
}

void QValueSlider::setValue( double v )
{
	blockSignals( true );
	spin_->setValue( v );
	slider_->setValue( to_int( v ) );
	blockSignals( false );
}

void QValueSlider::sliderAction( int i )
{
	//spin_->setValue( to_double( i ) );
	spin_->setValue( to_double( slider_->sliderPosition() ) );
}
