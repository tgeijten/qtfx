#pragma once

#include "QWidget"
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QTreeWidget"
#include "QSplitter"

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
	void refresh();

private:
	QSplitter* splitter;
	QTreeWidget* tree;
	QtCharts::QChart* chart;
	QtCharts::QChartView* chartView;
	QStorageDataModel* model;
};
