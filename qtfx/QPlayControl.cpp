#include "QPlayControl.h"

#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QBoxLayout>

#include "xo/system/log.h"

#include <math.h>

#pragma warning( disable: 26444 )

QPlayControl::QPlayControl( QWidget *parent ) :
QWidget( parent ),
currentTime( 0.0 ),
stepTime( 0.01 ),
pageTime( 0.1 ),
slomoFactor( 1.0 ),
minTime( 0.0 ),
maxTime( 1.0 ),
decimals_( 2 ),
autoExtendRange_( false ),
timer_delta( 0 )
{
	playButton = new QToolButton( this );
	playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
	connect( playButton, &QToolButton::clicked, this, &QPlayControl::togglePlay );

	resetButton = new QToolButton( this );
	resetButton->setIcon( style()->standardIcon( QStyle::SP_MediaSkipBackward ) );
	connect( resetButton, &QToolButton::clicked, this, &QPlayControl::reset );

	nextButton = new QToolButton( this );
	nextButton->setIcon( style()->standardIcon( QStyle::SP_MediaSeekForward) );
	nextButton->setStyleSheet( "border: 0px" );
	connect( nextButton, &QToolButton::clicked, this, &QPlayControl::stepForward );

	previousButton = new QToolButton( this );
	previousButton->setIcon( style()->standardIcon( QStyle::SP_MediaSeekBackward ) );
	previousButton->setStyleSheet( "border: 0px" );
	connect( previousButton, &QToolButton::clicked, this, &QPlayControl::stepBack );

	loopButton = new QToolButton( this );
	loopButton->setCheckable( true );
	loopButton->setIcon( style()->standardIcon( QStyle::SP_BrowserReload ) );

	lcdNumber = new QLCDNumber( this );
	lcdNumber->setDigitCount( 6 );
	lcdNumber->setSmallDecimalPoint( true );
	lcdNumber->setFrameStyle( QFrame::NoFrame );
	lcdNumber->setSegmentStyle( QLCDNumber::Flat );
	lcdNumber->display( "0.000" );

	slider = new QSlider( Qt::Horizontal, this );
	slider->setSingleStep( 10 );
	slider->setPageStep( 100 );
	connect( slider, &QSlider::valueChanged, this, &QPlayControl::updateSlider );
	connect( slider, &QSlider::sliderReleased, this, &QPlayControl::sliderReleased );

	slomoBox = new QComboBox( this );
	setSlomoRange( 2, -5 );
	connect( slomoBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateSlowMotion( int ) ) );

	QBoxLayout *lo = new QHBoxLayout;
	lo->setMargin( 0 );
	lo->setSpacing( 2 );
	lo->addWidget( playButton );
	lo->addWidget( resetButton );
	lo->addWidget( lcdNumber );
	lo->addWidget( previousButton );
	lo->addWidget( slider );
	lo->addWidget( nextButton );
	lo->addWidget( loopButton );
	lo->addWidget( slomoBox );
	setLayout( lo );

	connect( &qtimer, &QTimer::timeout, this, &QPlayControl::updateTime );
}

void QPlayControl::setSlomoRange( int max_power_of_2, int min_power_of_2 )
{
	slomoBox->clear();
	for ( int slomo = max_power_of_2; slomo >= min_power_of_2; --slomo )
	{
		QString label = slomo >= 0 ? QString::asprintf( "%d x", (int)pow( 2, slomo ) ) : QString::asprintf( "1/%d x", (int)pow( 2, -slomo ) );
		slomoBox->addItem( label, QVariant( pow( 2, slomo ) ) );
	}
	slomoBox->setCurrentIndex( max_power_of_2 );
}

void QPlayControl::setRange( double min, double max )
{
	minTime = min;
	maxTime = max;
	slider->setRange( static_cast< int >( 1000 * minTime + 0.5 ), static_cast< int >( 1000 * maxTime + 0.5 ) );
}

void QPlayControl::setTime( double time )
{
	if ( time > maxTime )
	{
		if ( autoExtendRange() ) // adjust maximum
		{
			setRange( minTime, time );
			currentTime = time;
		}
		else if ( isPlaying() && loop() ) // loop
		{
			currentTime = minTime;
		}
		else // clamp to maximum and stop playing
		{
			currentTime = maxTime;
			if ( isPlaying() )
				stop();
		}
	}
	else if ( time < minTime )
	{
		currentTime = minTime;
	}
	else currentTime = time;

	slider->blockSignals( true );
	slider->setValue( int( currentTime * 1000 ) );
	slider->blockSignals( false );

	lcdNumber->display( QString::asprintf( "%.*f", decimals_, currentTime ) );
	
	emit timeChanged( currentTime );
}

void QPlayControl::setDigits( int digits, int decimals )
{
	decimals_ = decimals;
	lcdNumber->setDigitCount( digits );
	lcdNumber->display( QString::asprintf( "%.*f", decimals_, currentTime ) );
}

bool QPlayControl::loop() const
{
	return loopButton->isChecked();
}

bool QPlayControl::isPlaying() const
{
	return qtimer.isActive();
}

void QPlayControl::setTimeStop( double time )
{
	stop();
	setTime( time );
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
		timer.restart();
		timer_delta( 0 );
		playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPause ) );
		emit playTriggered();
	}
}

void QPlayControl::stop()
{
	if ( isPlaying() )
	{
		qtimer.stop();
		updateTime();
		playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
		emit stopTriggered();
	}
}

void QPlayControl::stopReset()
{
	if ( isPlaying() )
		stop();
	else reset();
}

void QPlayControl::togglePlay()
{
	if ( isPlaying() )
		stop();
	else play();
}

void QPlayControl::reset()
{
	if ( isPlaying() )
		stop();
	setTime( minTime );
	emit resetTriggered();
}

void QPlayControl::end()
{
	if ( isPlaying() )
		stop();
	setTime( maxTime );
}

void QPlayControl::stepBack()
{
	setTime( currentTime - stepTime );
}

void QPlayControl::stepForward()
{
	setTime( currentTime + stepTime );
}

void QPlayControl::pageBack()
{
	setTime( currentTime - pageTime );
}

void QPlayControl::pageForward()
{
	setTime( currentTime + pageTime );
}

void QPlayControl::faster()
{
	slomoBox->setCurrentIndex( std::max( 0, slomoBox->currentIndex() - 1 ) );
}

void QPlayControl::slower()
{
	slomoBox->setCurrentIndex( std::min( slomoBox->count() - 1, slomoBox->currentIndex() + 1 ) );
}

void QPlayControl::updateSlowMotion( int idx )
{
	slomoFactor = slomoBox->itemData( idx ).toDouble();
	emit slowMotionChanged( slomoBox->itemData( idx ).toInt() );
}

void QPlayControl::updateSlider( int value )
{
	setTimeStop( value / 1000.0 );
	emit sliderChanged( value );
}

void QPlayControl::updateTime()
{
	setTime( currentTime + slomoFactor * timer_delta( timer().secondsd() ) );
}
