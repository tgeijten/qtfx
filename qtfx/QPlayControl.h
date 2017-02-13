#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QLCDNumber>
#include <QTimer>
#include "flut/timer.hpp"
#include "flut/math/delta.hpp"

class QAbstractButton;
class QAbstractSlider;
class QComboBox;

class QPlayControl : public QWidget
{
	Q_OBJECT

public:
	QPlayControl( QWidget *parent = 0 );
	float slowMotion() const;

	void setRange( double min, double max );
	void setPlaying( bool play );
	void setTime( double time );
	bool getLoop();

public slots:
	void setLoop( bool loop );
	void play();
	void stop();
	void reset();
	void previous();
	void next();

signals:
	void playTriggered();
	void stopTriggered();
	void resetTriggered();
	void nextTriggered();
	void previousTriggered();
	void slowMotionChanged( int i );
	void timeChanged( double time );
	void sliderChanged( int );

private slots:
	void updateSlowMotion( int );
	void updateSlider( int );
	void timeout();

private:
	QAbstractButton *playButton;
	QAbstractButton *stopButton;
	QAbstractButton *nextButton;
	QAbstractButton *previousButton;
	QAbstractButton *loopButton;
	QComboBox *slowMotionBox;
	QSlider *slider;
	QLCDNumber* label;
	double currentTime;
	double skipTime;
	double slomoFactor;
	bool loop;

	QTimer qtimer;
	flut::timer timer;
	flut::delta< flut::timer::seconds_t > timer_delta;
};