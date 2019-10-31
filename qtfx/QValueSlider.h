#pragma once

#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>

class QValueSlider : public QWidget
{
	Q_OBJECT

public:
	QValueSlider( QWidget* parent = nullptr, int margin = 0, int spacing = 4 );
	void setRange( double min, double max );
	void setValue( double v ) { spin_->setValue( v ); }
	double value() { return spin_->value(); }

signals:
	void valueChanged( double d );

private slots:
	void spinValueChanged( double d );
	void sliderAction( int i );

private:
	int to_int( double d ) { return int( 100.0 * d ); }
	double to_double( int i ) { return 0.01 * i; }

	QDoubleSpinBox* spin_;
	QLabel* min_;
	QLabel* max_;
	QSlider* slider_;
};
