#pragma once

#include "QWidget"
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QSplitter"
#include "QTreeWidget"

class QDataAnalysisModel
{
public:
	QDataAnalysisModel() {}
	virtual ~QDataAnalysisModel() {}

	virtual size_t getSize() = 0;
	virtual QString getLabel( int idx ) = 0;
	virtual double getValue( int idx, double time ) = 0;
	virtual QtCharts::QLineSeries* getSeries( int idx ) = 0;
};

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}
	void refresh( double time, bool refreshAll = true );

private:
	void reset();

	int smallRefreshItemCount = 4;
	int currentUpdateIdx;
	QSplitter* splitter;
	QTreeWidget* itemList;
	QtCharts::QChart* chart;
	QtCharts::QChartView* chartView;
	QDataAnalysisModel* model;
};
