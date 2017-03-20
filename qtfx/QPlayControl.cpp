#include "QPlayControl.h"

#include <QToolButton>
#include <QWidget>
#include <QComboBox>
#include "flut/system/log.hpp"

QPlayControl::QPlayControl( QWidget *parent /*= 0 */ ) :
currentTime( 0.0 ),
skipTime( 0.01 ),
slomoFactor( 1.0 ),
minTime( 0.0 ),
maxTime( 1.0 ),
autoExtendRange( false )
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
	connect( slider, SIGNAL( sliderReleased() ), this, SIGNAL( sliderReleased() ) );

	slowMotionBox = new QComboBox( this );
	for ( int slomo = 2; slomo >= -5; --slomo )
	{
		QString label = slomo >= 0 ? QString().sprintf( "%d x", (int)pow( 2, slomo ) ) : QString().sprintf( "1/%d x", (int)pow( 2, -slomo ) );
		slowMotionBox->addItem( label, QVariant( pow( 2, slomo ) ) );
	}
	slowMotionBox->setCurrentIndex( 2 );
	connect( slowMotionBox, SIGNAL( activated( int ) ), SLOT( updateSlowMotion( int ) ) );

	QBoxLayout *lo = new QHBoxLayout;
	lo->setMargin( 0 );
	lo->setSpacing( 2 );
	lo->addWidget( previousButton );
	lo->addWidget( playButton );
	lo->addWidget( stopButton );
	lo->addWidget( nextButton );
	lo->addWidget( label );
	lo->addWidget( slider );
	lo->addWidget( loopButton );
	lo->addWidget( slowMotionBox );
	setLayout( lo );

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
		if ( getAutoExtendRange() )
		{
			// adjust maximum
			setRange( minTime, currentTime );
		}
		else if ( getLoop() )
		{
			// restart
			currentTime = minTime;
		}
		else
		{
			// stop playing
			currentTime = maxTime;
			stop();
		}
	}

	slider->blockSignals( true );
	slider->setValue( int( currentTime * 1000 ) );
	slider->blockSignals( false );

	label->display( QString().sprintf( "%.2f", currentTime ) );
	
	emit timeChanged( currentTime );
}

bool QPlayControl::getLoop()
{
	return loopButton->isChecked();
}

bool QPlayControl::isPlaying()
{
	return qtimer.isActive();
}

void QPlayControl::setLoop( bool b )
{
	loopButton->setChecked( b );
}

void QPlayControl::play()
{
	if ( !isPlaying() )
	{
		if ( currentTime >= maxTime )
			reset();
		qtimer.start( 10 );
		timer.reset();
		timer_delta.reset();
		emit playTriggered();
	}
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

void QPlayControl::toggle()
{
	if ( isPlaying() )
		stop();
	else play();
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
	emit previousTriggered();
}

void QPlayControl::next()
{
	setTime( currentTime + skipTime );
	emit nextTriggered();
}

void QPlayControl::updateSlowMotion( int idx )
{
	slomoFactor = slowMotionBox->itemData( idx ).toDouble();
	emit slowMotionChanged( slowMotionBox->itemData( idx ).toInt() );
}

void QPlayControl::updateSlider( int value )
{
	setTime( value / 1000.0 );
	emit sliderChanged( value );
}

void QPlayControl::timeout()
{
	setTime( currentTime + slomoFactor * timer_delta( timer.seconds() ) );
}
