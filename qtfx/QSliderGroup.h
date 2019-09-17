#pragma once

#include "QWidget"
#include <vector>

class QSliderGroup : public QWidget
{
	Q_OBJECT

public:
	QSliderGroup( QWidget* parent = nullptr, int margin = 4, int spacing = 4 );

	QSlider* addSlider( const QString& name );
	void clear();

	std::vector< QSlider* > sliders;
};
