#pragma once

#if defined QTFX_USE_QCUSTOMPLOT
#include "qcustomplot.h"
#else
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#endif
#include <vector>

class QPlot : public QWidget
{
public:
	QPlot( QWidget* parent = 0 );
	virtual ~QPlot();
		
private:
#if defined QTFX_USE_QCUSTOMPLOT
	QCustomPlot* customPlot;
	QCPItemLine* customPlotLine;
	flut::flat_map< int, QCPGraph* > series;
#else
	QtCharts::QChart* chart;
	std::vector< QtCharts::QLineSeries* > series;
	QtCharts::QChartView* chartView;
#endif
};
