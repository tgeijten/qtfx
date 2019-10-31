#pragma once

#include "QWidget"
#include <vector>
#include "QValueSlider.h"

class QSliderGroup : public QWidget
{
	Q_OBJECT

public:
	QSliderGroup( QWidget* parent = nullptr, int margin = 4, int spacing = 4 );

	QValueSlider* addSlider( const QString& name );
	void clear();

	std::vector< QValueSlider* > sliders;
};
