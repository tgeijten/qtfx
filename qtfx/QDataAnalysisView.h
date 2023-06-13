#pragma once

#include "xo/container/flat_map.h"
#include "xo/container/storage.h"
#include "xo/container/sorted_vector.h"

#include <QWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QLineEdit>
#include <QGroup.h>
#include <QCheckBox>
#include <QPushButton>

#include "QDataAnalysisModel.h"

class QCPRange;
class QCustomPlot;
class QCPItemLine;
class QCPGraph;

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel& m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}

	void setTime( double time, bool refreshAll = true );
	void reloadData();
	void setRange( double lower, double upper );
	void setLineWidth( float f ) { lineWidth = f; }
	void setAutoFitVerticalAxis( bool b ) { autoFitVerticalAxis = b; }
	void setFilterText( const QString& str  ) { filter->setText( str ); }
	QLineEdit* filterWidget() { return filter; }
	QVGroup* itemGroupWidget() { return itemGroup; }

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
	void fitVerticalAxis();
	int decimalPoints( double v );

	enum SeriesStyle { noStyle, lineStyle, discStyle };
	SeriesStyle seriesStyle = noStyle;
	size_t maxSeriesCount = 20;
	int smallRefreshItemCount = 100;
	float averageFrameDuration = 0.0f;
	float lineWidth = 1.0f;
	bool autoFitVerticalAxis = false;
	float minDataPointsVisible = 8;

	int currentUpdateIdx;
	double currentTime;
	QCheckBox* selectBox;
	QLineEdit* filter;
	QGroup* filterGroup;
	QSplitter* splitter;
	QVGroup* itemGroup;
	QTreeWidget* itemList;
	QPushButton* keepButton;
	QDataAnalysisModel& model;

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
