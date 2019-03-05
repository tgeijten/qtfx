#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLCDNumber>
#include <QTimer>
#include "xo/time/timer.h"
#include "xo/numerical/delta.h"
#include "xo/utility/types.h"

class QAbstractSlider;
class QToolButton;
class QComboBox;

class QPlayControl : public QWidget
{
	Q_OBJECT

public:
	QPlayControl( QWidget *parent = 0 );


	double slowMotionFactor() const { return slomoFactor; }
	void setSlomoRange( int max_power_of_2, int min_power_of_2 );

	void setRange( double min, double max );
	void setTime( double time );
	void setStepTime( double t ) { stepTime = t; }
	void setPageTime( double t ) { pageTime = t; }
	void setDigits( int digits, int decimals );
	bool loop() const;
	bool autoExtendRange() const { return autoExtendRange_;  }
	bool isPlaying() const;

public slots:
	void setLoop( bool b );
	void setAutoExtendRange( bool b ) { autoExtendRange_ = b; }
	void play();
	void stop();
	void reset();
	void stopReset();
	void end();
	void togglePlay();
	void toggleLoop() { setLoop( !loop() ); }
	void stepBack();
	void stepForward();
	void pageBack();
	void pageForward();
	void faster();
	void slower();

signals:
	void playTriggered();
	void stopTriggered();
	void resetTriggered();
	void nextTriggered();
	void previousTriggered();
	void slowMotionChanged( int );
	void timeChanged( double );
	void sliderChanged( int );
	void sliderReleased();

private slots:
	void updateSlider( int );
	void updateSlowMotion( int );
	void updateTime();

private:
	QToolButton *playButton;
	QToolButton *resetButton;
	QToolButton *nextButton;
	QToolButton *previousButton;
	QToolButton *loopButton;
	QComboBox *slomoBox;
	QSlider *slider;
	QLCDNumber* lcdNumber;
	double currentTime;
	double stepTime;
	double pageTime;
	double slomoFactor;
	double minTime, maxTime;
	int decimals_;

	bool autoExtendRange_;

	QTimer qtimer;
	xo::timer timer;
	xo::delta< xo::seconds_t > timer_delta;
};
