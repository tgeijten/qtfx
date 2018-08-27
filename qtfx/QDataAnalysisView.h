#pragma once

#include "xo/container/flat_map.h"
#include "xo/container/storage.h"

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
#include "xo/container/sorted_vector.h"

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
	void rangeChanged( const QCPRange &newRange, const QCPRange &oldRange );
	void filterChanged( const QString& filter );
	void setSelectionState( int state );
	void select( int state ) { setSelectionState( state ); }
	void selectAll() { setSelectionState( Qt::Checked ); }
	void selectNone() { setSelectionState( Qt::Unchecked ); }

signals:
	void timeChanged( double );

private:
	QColor getStandardColor( int idx );
	void addSeries( int idx );
	void removeSeries( int idx );
	void updateIndicator();
	void updateFilter();
	void updateSelectBox();
	int decimalPoints( double v );

	size_t maxSeriesCount = 20;
	int smallRefreshItemCount = 100;
	float minSeriesInterval = 0.01f;
	float currentSeriesInterval = 0;
	int currentUpdateIdx;
	double currentTime;
	QCheckBox* selectBox;
	QLineEdit* filter;
	QSplitter* splitter;
	QGroup* itemGroup;
	QTreeWidget* itemList;
	QDataAnalysisModel* model;

#if !defined QTFX_NO_QCUSTOMPLOT
	QCustomPlot* customPlot;
	QCPItemLine* customPlotLine;
	xo::sorted_vector< int > freeColors;

	struct Series {
		int channel;
		int color;
		QCPGraph* graph;
	};
	std::vector< Series > series;

	xo::sorted_vector< QString > persistentSerieNames;
#else
	QtCharts::QChart* chart;
	xo::flat_map< int, QtCharts::QLineSeries* > series;
	QtCharts::QChartView* chartView;
#endif
	void refreshSeriesStyle();
};
