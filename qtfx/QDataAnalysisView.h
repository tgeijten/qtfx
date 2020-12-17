#pragma once

#include "xo/container/flat_map.h"
#include "xo/container/storage.h"

#include "QWidget"
#include "QSplitter"
#include "QTreeWidget"
#include "QDataAnalysisModel.h"

#include "QLineEdit"
#include "QGroup.h"
#include "QCheckBox"
#include "QToolButton"
#include "xo/container/sorted_vector.h"

class QCPRange;
class QCustomPlot;
class QCPItemLine;
class QCPGraph;

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}
	void refresh( double time, bool refreshAll = true );
	void reset();

	void setLineWidth( float f ) { lineWidth = f; }
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
	void holdSeries();
	void focusFilterEdit() { show(); filter->setFocus(); }

signals:
	void timeChanged( double );

private:
	void addSeries( int idx );
	void removeSeries( int idx );
	void updateIndicator();
	void updateFilter();
	void updateSelectBox();
	int decimalPoints( double v );

	enum SeriesStyle { noStyle, lineStyle, discStyle };
	SeriesStyle seriesStyle = noStyle;
	size_t maxSeriesCount = 20;
	int smallRefreshItemCount = 100;
	float minSeriesInterval = 0.01f;
	float currentSeriesInterval = 0;
	float lineWidth = 1.5f;
	int currentUpdateIdx;
	double currentTime;
	QCheckBox* selectBox;
	QLineEdit* filter;
	QSplitter* splitter;
	QGroup* itemGroup;
	QTreeWidget* itemList;
	QDataAnalysisModel* model;

	QCustomPlot* customPlot;
	QCPItemLine* customPlotLine;
	xo::sorted_vector< int > freeColors;

	struct Series {
		int channel;
		int color;
		QCPGraph* graph;
	};
	std::vector< Series > series;
	std::vector< QCPGraph* > heldSeries;
	xo::sorted_vector< QString > persistentSerieNames;

	void updateSeriesStyle();
};
