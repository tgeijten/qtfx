#pragma once

#if defined QTFX_USE_QCUSTOMPLOT
#include "qcustomplot.h"
#else
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QtWidgets/QGraphicsLayout"
#endif
#include <vector>
#include "flut/system/assert.hpp"

class QPlot : public QWidget
{
public:
	QPlot( QWidget* parent = 0 );
	virtual ~QPlot();

	size_t addSeries( const QString& label );
		
private:

#if defined QTFX_USE_QCUSTOMPLOT
	QCustomPlot* customPlot;
#else
	QtCharts::QChart* chart;
	QtCharts::QChartView* chartView;
#endif
};
