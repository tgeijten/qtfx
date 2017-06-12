#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLCDNumber>
#include <QTimer>
#include "flut/timer.hpp"
#include "flut/math/delta.hpp"
#include "flut/system/types.hpp"

class QAbstractButton;
class QAbstractSlider;
class QComboBox;

class QPlayControl : public QWidget
{
	Q_OBJECT

public:
	QPlayControl( QWidget *parent = 0 );
	double slowMotionFactor() const { return slomoFactor; }

	void setRange( double min, double max );
	void setTime( double time );
	bool getLoop();
	bool getAutoExtendRange() { return autoExtendRange;  }
	bool isPlaying();

public slots:
	void setLoop( bool b );
	void setAutoExtendRange( bool b ) { autoExtendRange = b; }
	void play();
	void stop();
	void reset();
	void toggle();
	void previous();
	void next();
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
	void timeout();

private:
	QAbstractButton *playButton;
	QAbstractButton *stopButton;
	QAbstractButton *nextButton;
	QAbstractButton *previousButton;
	QAbstractButton *loopButton;
	QComboBox *slomoBox;
	QSlider *slider;
	QLCDNumber* label;
	double currentTime;
	double skipTime;
	double slomoFactor;
	double minTime, maxTime;

	bool autoExtendRange;

	QTimer qtimer;
	flut::timer timer;
	flut::delta< flut::seconds_t > timer_delta;
};
