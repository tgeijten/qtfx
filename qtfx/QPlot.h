#pragma once

#if defined QTFX_USE_QCUSTOMPLOT
#include "qcustomplot.h"
#else
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#endif
#include <vector>
#include "flut/system/assert.hpp"

class QPlot : public QWidget
{
public:
	QPlot( QWidget* parent = 0 );
	virtual ~QPlot();

	template< typename IX, typename IY > size_t addSeries( const QString& label, IX beginx, IX endx, IY beginy, IY endy ) {
#if defined QTFX_USE_QCUSTOMPLOT
		FLUT_NOT_IMPLEMENTED;
#else
#endif
	}
		
private:
#if defined QTFX_USE_QCUSTOMPLOT
	QCustomPlot* customPlot;
#else
	QtCharts::QChart* chart;
	QtCharts::QChartView* chartView;
#endif

};
