#pragma once

#define QTFX_USE_QCUSTOMPLOT

#include "flut/vecmap.hpp"
#include "flut/buffer/storage.hpp"

#include "QWidget"
#include "QSplitter"
#include "QTreeWidget"
#include "QDataAnalysisModel.h"

#if defined QTFX_USE_QCUSTOMPLOT
#include "qcustomplot.h"
#else
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#endif

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}
	void refresh( double time, bool refreshAll = true );

public slots:
	void itemChanged( QTreeWidgetItem* item, int column );
	void updateAllSeries();
	void updateSeries( int index );

private:
	QColor getStandardColor( int idx );
	void reset();
	void addSeries( int idx );
	void removeSeries( int idx );
	void updateIndicator();

	int smallRefreshItemCount = 4;
	double minSeriesInterval = 0.01;
	int currentUpdateIdx;
	double currentTime;
	QSplitter* splitter;
	QTreeWidget* itemList;
	QDataAnalysisModel* model;

#if defined QTFX_USE_QCUSTOMPLOT
	QCustomPlot* customPlot;
	QCPItemLine* customPlotLine;
	flut::vecmap< int, QCPGraph* > series;
#else
	QtCharts::QChart* chart;
	flut::vecmap< int, QtCharts::QLineSeries* > series;
	QtCharts::QChartView* chartView;
#endif
};
