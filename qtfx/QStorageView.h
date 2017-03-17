#pragma once

#include "QWidget"
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QSplitter"
#include "QTreeWidget"

class QStorageDataModel
{
public:
	QStorageDataModel() {}
	virtual ~QStorageDataModel() {}

	virtual size_t getSize() = 0;
	virtual QString getLabel( int idx ) = 0;
	virtual double getValue( int idx, double time ) = 0;
	virtual QtCharts::QLineSeries* getSeries( int idx ) = 0;
};

class QStorageView : public QWidget
{
	Q_OBJECT

public:
	QStorageView( QStorageDataModel* m, QWidget* parent = 0 );
	virtual ~QStorageView() {}
	void refresh( double time, bool refreshAll = true );

private:
	void reset();

	int smallRefreshItemCount = 4;
	int currentUpdateIdx;
	QSplitter* splitter;
	QTreeWidget* itemList;
	QtCharts::QChart* chart;
	QtCharts::QChartView* chartView;
	QStorageDataModel* model;
};
