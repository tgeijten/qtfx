#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>

class QValueSlider : public QWidget
{
	Q_OBJECT

public:
	QValueSlider( QWidget* parent = nullptr, double stepSize = 0.01, int margin = 0, int spacing = 4 );
	void setRange( double min, double max );
	void setValue( double v );
	double value() { return spin_->value(); }

signals:
	void valueChanged( double d );

private slots:
	void spinValueChanged( double d );
	void sliderAction( int i );

private:
	int to_int( double d ) { return int( d / stepSize_ ); }
	double to_double( int i ) { return stepSize_ * i; }

	QDoubleSpinBox* spin_;
	QLabel* min_;
	QLabel* max_;
	QSlider* slider_;
	double stepSize_;
};
