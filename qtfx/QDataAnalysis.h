#pragma once

#include "QWidget"
#include "QtCharts/QChart"
#include "QtCharts/QLineSeries"
#include "QtCharts/QChartView"
#include "QSplitter"
#include "QTreeWidget"
#include "flut/vecmap.hpp"
#include "flut/buffer/storage.hpp"

class QDataAnalysisModel
{
public:
	QDataAnalysisModel() {}
	virtual ~QDataAnalysisModel() {}
	virtual size_t getVariableCount() const = 0;
	virtual double getTimeStart() const { return 0.0; }
	virtual double getTimeFinish() const { return 0.0; }
	virtual QString getLabel( int idx ) const = 0;
	virtual double getValue( int idx, double time ) const = 0;
	virtual std::vector< std::pair< float, float > > getSeries( int idx, double min_interval = 0.0 ) const = 0;
};

template< typename T >
class StorageDataAnalysisModel : public QDataAnalysisModel
{
public:
	StorageDataAnalysisModel( const flut::storage< T >& s ) : storage( s ) {}
	const flut::storage< T >& storage;
	virtual QString getLabel( int idx ) const override { return QString( storage.get_label( idx ).c_str() ); }
	virtual std::vector<std::pair<float, float>> getSeries( int idx, double min_interval = 0.0 ) const override {
		throw std::logic_error( "The method or operation is not implemented." );
	}

	virtual double getTimeFinish() const override { return storage.empty() ? 0.0 : storage( storage.frame_size() - 1, 0 ); }
	virtual double getTimeStart() const override { return storage.empty() ? 0.0 : storage( 0, 0 ); }
	virtual double getValue( int idx, double time ) const override
	{
		throw std::logic_error( "The method or operation is not implemented." );
	}

	virtual size_t getVariableCount() const override { return storage.channel_size(); }
};

class QDataAnalysisView : public QWidget
{
	Q_OBJECT

public:
	QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent = 0 );
	virtual ~QDataAnalysisView() {}
	void refresh( double time, bool refreshAll = true );

public slots:
	void itemChanged( QTreeWidgetItem* item, int column );
	void updateSeries();
	void updateItemSeries( QTreeWidgetItem* item );

private:
	void reset();

	int smallRefreshItemCount = 4;
	double minSeriesInterval = 0.01;
	int currentUpdateIdx;
	QSplitter* splitter;
	QTreeWidget* itemList;
	QtCharts::QChart* chart;
	flut::vecmap< int, QtCharts::QLineSeries* > series;
	QtCharts::QChartView* chartView;
	QDataAnalysisModel* model;
};
