#include "QPlayControl.h"

#include <QToolButton>
#include <QWidget>
#include <QComboBox>

QPlayControl::QPlayControl( QWidget *parent /*= 0 */ ) :
currentTime( 0.0 ),
skipTime( 0.01 ),
slomoFactor( 1.0 ),
minTime( 0.0 ),
maxTime( 1.0 ),
loop( false )
{
	playButton = new QToolButton( this );
	playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
	connect( playButton, SIGNAL( clicked() ), this, SLOT( play() ) );

	stopButton = new QToolButton( this );
	stopButton->setIcon( style()->standardIcon( QStyle::SP_MediaStop ) );
	connect( stopButton, SIGNAL( clicked() ), this, SLOT( stop() ) );

	nextButton = new QToolButton( this );
	nextButton->setIcon( style()->standardIcon( QStyle::SP_MediaSkipForward ) );
	connect( nextButton, SIGNAL( clicked() ), this, SLOT( next() ) );

	previousButton = new QToolButton( this );
	previousButton->setIcon( style()->standardIcon( QStyle::SP_MediaSkipBackward ) );
	connect( previousButton, SIGNAL( clicked() ), this, SLOT( previous() ) );

	loopButton = new QToolButton( this );
	loopButton->setCheckable( true );
	loopButton->setIcon( style()->standardIcon( QStyle::SP_BrowserReload ) );

	label = new QLCDNumber( this );
	label->setDigitCount( 5 );
	label->setSmallDecimalPoint( true );
	label->setFrameStyle( QFrame::Box );
	label->setSegmentStyle( QLCDNumber::Flat );
	label->display( "0.00" );

	slider = new QSlider( Qt::Horizontal, this );
	slider->setSingleStep( 10 );
	slider->setPageStep( 100 );
	connect( slider, SIGNAL( valueChanged( int ) ), this, SLOT( updateSlider( int ) ) );

	slowMotionBox = new QComboBox( this );
	slowMotionBox->addItem( "1 x", QVariant( 1 ) );
	slowMotionBox->addItem( "1/2 x", QVariant( 2 ) );
	slowMotionBox->addItem( "1/4 x", QVariant( 4 ) );
	slowMotionBox->addItem( "1/8 x", QVariant( 8 ) );
	slowMotionBox->setCurrentIndex( 0 );
	connect( slowMotionBox, SIGNAL( activated( int ) ), SLOT( updateSlowMotion( int ) ) );

	QBoxLayout *layout = new QHBoxLayout;
	layout->setMargin( 0 );
	layout->setSpacing( 2 );
	layout->addWidget( previousButton );
	layout->addWidget( playButton );
	layout->addWidget( stopButton );
	layout->addWidget( nextButton );
	layout->addWidget( label );
	layout->addWidget( slider );
	layout->addWidget( loopButton );
	layout->addWidget( slowMotionBox );
	setLayout( layout );

	connect( &qtimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
}

void QPlayControl::setRange( double min, double max )
{
	minTime = min;
	maxTime = max;
	slider->setRange( static_cast< int >( 1000 * minTime + 0.5 ), static_cast< int >( 1000 * maxTime + 0.5 ) );
}

void QPlayControl::setTime( double time )
{
	currentTime = time;

	if ( currentTime > maxTime )
	{
		if ( getLoop() )
		{
			currentTime = minTime;
		}
		else
		{
			currentTime = maxTime;
			stop();
		}
	}

	slider->setValue( int( currentTime * 1000 ) );
	label->display( QString().sprintf( "%.2f", currentTime ) );
	
	emit timeChanged( currentTime );
}

bool QPlayControl::getLoop()
{
	return loopButton->isChecked();
}

void QPlayControl::setLoop( bool l )
{
	loop = l;
}

void QPlayControl::play()
{
	qtimer.start( 10 );
	timer.reset();
	timer_delta.reset();
	emit playTriggered();
}

void QPlayControl::stop()
{
	if ( qtimer.isActive() )
	{
		qtimer.stop();
		emit stopTriggered();
	}
	else reset();
}

void QPlayControl::reset()
{
	if ( qtimer.isActive() )
		qtimer.stop();

	setTime( 0 );
	emit resetTriggered();
}

void QPlayControl::previous()
{
	setTime( currentTime - skipTime );
}

void QPlayControl::next()
{
	setTime( currentTime + skipTime );
}

void QPlayControl::updateSlowMotion( int idx )
{
	slomoFactor = 1.0 / slowMotionBox->itemData( idx ).toDouble();
	emit slowMotionChanged( slowMotionBox->itemData( idx ).toInt() );
}

void QPlayControl::updateSlider( int value )
{
	setTime( value / 1000.0 );
	emit timeChanged( currentTime );
	emit sliderChanged( value );
}

void QPlayControl::timeout()
{
	setTime( currentTime + slomoFactor * timer_delta( timer.seconds() ) );
}
