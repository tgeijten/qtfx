#include "QPlayControl.h"

#include <QToolButton>
#include <QWidget>
#include <QComboBox>
#include "xo/system/log.h"

QPlayControl::QPlayControl( QWidget *parent ) :
QWidget( parent ),
currentTime( 0.0 ),
stepTime( 0.01 ),
pageTime( 0.1 ),
slomoFactor( 1.0 ),
minTime( 0.0 ),
maxTime( 1.0 ),
autoExtendRange_( false ),
timer_delta( 0 )
{
	playButton = new QToolButton( this );
	playButton->setIcon( style()->standardIcon( QStyle::SP_MediaPlay ) );
	connect( playButton, SIGNAL( clicked() ), this, SLOT( play() ) );

	stopButton = new QToolButton( this );
	stopButton->setIcon( style()->standardIcon( QStyle::SP_MediaStop ) );
	connect( stopButton, SIGNAL( clicked() ), this, SLOT( stopReset() ) );

	nextButton = new QToolButton( this );
	nextButton->setIcon( style()->standardIcon( QStyle::SP_MediaSkipForward ) );
	connect( nextButton, SIGNAL( clicked() ), this, SLOT( stepForward() ) );

	previousButton = new QToolButton( this );
	previousButton->setIcon( style()->standardIcon( QStyle::SP_MediaSkipBackward ) );
	connect( previousButton, SIGNAL( clicked() ), this, SLOT( stepBack() ) );

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

	slomoBox = new QComboBox( this );
	for ( int slomo = 2; slomo >= -5; --slomo )
	{
		QString label = slomo >= 0 ? QString().sprintf( "%d x", (int)pow( 2, slomo ) ) : QString().sprintf( "1/%d x", (int)pow( 2, -slomo ) );
		slomoBox->addItem( label, QVariant( pow( 2, slomo ) ) );
	}
	slomoBox->setCurrentIndex( 2 );
	connect( slomoBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateSlowMotion( int ) ) );

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
	lo->addWidget( slomoBox );
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

	label->display( QString().sprintf( "%.2f", currentTime ) );
	
	emit timeChanged( currentTime );
}

bool QPlayControl::loop() const
{
	return loopButton->isChecked();
}

bool QPlayControl::isPlaying() const
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
		timer_delta( 0 );
		emit playTriggered();
	}
}

void QPlayControl::stop()
{
	qtimer.stop();
	emit stopTriggered();
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
	setTime( value / 1000.0 );
	emit sliderChanged( value );
}

void QPlayControl::timeout()
{
	setTime( currentTime + slomoFactor * timer_delta( timer.seconds() ) );
}
