#pragma once

#include "flut/flat_map.hpp"
#include "flut/storage.hpp"

#include "QWidget"
#include "QSplitter"
#include "QTreeWidget"
#include "QDataAnalysisModel.h"

#if !defined QTFX_NO_QCUSTOMPLOT
#include "qcustomplot/qcustomplot.h"
#else
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QtWidgets/QGraphicsLayout"
#endif
#include "QLineEdit"
#include "QGroup.h"
#include "QCheckBox"
#include "QToolButton"

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}
	void refresh( double time, bool refreshAll = true );
	void reset();

	void setMinSeriesInterval( float f ) { minSeriesInterval = f; }

public slots:
	void itemChanged( QTreeWidgetItem* item, int column );
	void clearSeries();
	void updateSeries( int index );
	void mouseEvent( QMouseEvent* m );
	void filterChanged( const QString& filter );
	void setSelectionState( int state );
	void selectAll() { setSelectionState( Qt::Checked ); }
	void selectNone() { setSelectionState( Qt::Unchecked ); }

signals:
	void timeChanged( double );

private:
	QColor getStandardColor( int idx, float brightness = 0.75f );
	void addSeries( int idx );
	void removeSeries( int idx );
	void updateIndicator();
	void updateFilter();

	int smallRefreshItemCount = 100;
	float minSeriesInterval = 0.01f;
	int currentUpdateIdx;
	double currentTime;
	QToolButton* selectAllButton;
	QToolButton* selectNoneButton;
	QLineEdit* filter;
	QSplitter* splitter;
	QGroup* itemGroup;
	QTreeWidget* itemList;
	QDataAnalysisModel* model;

#if !defined QTFX_NO_QCUSTOMPLOT
	QCustomPlot* customPlot;
	QCPItemLine* customPlotLine;
	flut::flat_map< int, QCPGraph* > series;
#else
	QtCharts::QChart* chart;
	flut::flat_map< int, QtCharts::QLineSeries* > series;
	QtCharts::QChartView* chartView;
#endif
};
