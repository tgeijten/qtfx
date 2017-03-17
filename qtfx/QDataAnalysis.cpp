#include "QDataAnalysis.h"
#include "QAction"
#include "QHeaderView"
#include "QtWidgets/QGraphicsLayout"
#include <algorithm>
#include "flut/system/log.hpp"

using namespace QtCharts;

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent ) : QWidget( parent ), model( m ), currentUpdateIdx( 0 )
{
	itemList = new QTreeWidget( this );
	itemList->setRootIsDecorated( false );
	itemList->setColumnCount( 2 );
	itemList->header()->close();
	itemList->resize( 100, 100 );

	QStringList headerLabels;
	headerLabels << "Variable" << "Value";
	itemList->setHeaderLabels( headerLabels );
	itemList->header()->setStretchLastSection( false );
	for ( int i = 0; i < itemList->columnCount(); ++i )
		itemList->header()->setSectionResizeMode( i, i == 0 ? QHeaderView::Stretch : QHeaderView::ResizeToContents );

	connect( itemList, &QTreeWidget::itemChanged, this, &QDataAnalysisView::itemChanged );

	chart = new QChart();
	chart->setBackgroundRoundness( 0 );
	chart->setMargins( QMargins( 4, 4, 4, 4 ) );
	chart->layout()->setContentsMargins( 0, 0, 0, 0 );
	chart->legend()->setAlignment( Qt::AlignRight );
	chart->createDefaultAxes();
	chartView = new QChartView( chart, this );
	chartView->setContentsMargins( 0, 0, 0, 0 );
	chartView->setRenderHint( QPainter::Antialiasing );
	chartView->setRubberBand( QChartView::RectangleRubberBand );
	chartView->setBackgroundBrush( QBrush( Qt::red ) );
	chartView->resize( 300, 100 );

	splitter = new QSplitter( this );
	splitter->setContentsMargins( 0, 0, 0, 0 );
	splitter->addWidget( itemList );
	splitter->addWidget( chartView );
	splitter->setSizes( QList< int >{ 100, 300 } );
	splitter->setObjectName( "Analysis.Splitter" );

	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setContentsMargins( 4, 4, 4, 4 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );
}

void QDataAnalysisView::refresh( double time, bool refreshAll )
{
	if ( itemList->topLevelItemCount() != model->getVariableCount() )
		return reset();

	// update values
	if ( model->getVariableCount() > 0 )
	{
		if ( isVisible() )
		{
			int itemCount = refreshAll ? model->getVariableCount() : std::min<int>( smallRefreshItemCount, model->getVariableCount() );
			for ( size_t i = 0; i < itemCount; ++i )
			{
				itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString().sprintf( "%.3f", model->getValue( currentUpdateIdx, time ) ) );
				++currentUpdateIdx %= model->getVariableCount();
			}
		}
	}

	// TODO: update marker (from callout example)
	auto pos = chart->mapToPosition( QPointF( time, 0 ) );
}

void QDataAnalysisView::itemChanged( QTreeWidgetItem* item, int column )
{
	if ( column == 0 )
		updateItemSeries( item );
}

void QDataAnalysisView::updateSeries()
{
	if ( itemList->topLevelItemCount() != model->getVariableCount() )
		return reset();

	for ( int i = 0; i < model->getVariableCount(); ++i )
		updateItemSeries( itemList->topLevelItem( i ) );
}

void QDataAnalysisView::updateItemSeries( QTreeWidgetItem* item )
{
	auto idx = itemList->indexOfTopLevelItem( item );
	auto series_it = series.find( idx );

	if ( item->checkState( 0 ) && series_it == series.end() )
	{
		// create new series
		//flut::log::debug( "Adding series for " + item->text( 0 ).toStdString() );
		QLineSeries* ls = new QLineSeries;
		ls->setName( model->getLabel( idx ) );
		auto data = model->getSeries( idx, minSeriesInterval );
		for ( auto& e : data )
			ls->append( e.first, e.second );
		chart->addSeries( ls );
		chart->createDefaultAxes();
		chart->zoomReset();
		series.emplace_back( idx, ls );
	}
	else if ( series_it != series.end() && !item->checkState( 0 ) )
	{
		// remove series
		//flut::log::debug( "Removing series for " + item->text( 0 ).toStdString() );
		chart->removeSeries( series_it->second );
		chart->zoomReset();
		chart->createDefaultAxes();
		series.erase( series_it );
	}
}

void QDataAnalysisView::reset()
{
	itemList->clear();
	for ( size_t i = 0; i < model->getVariableCount(); ++i )
	{
		auto* wdg = new QTreeWidgetItem( itemList, QStringList( model->getLabel( i ) ) );
		wdg->setTextAlignment( 1, Qt::AlignRight );
		wdg->setFlags( wdg->flags() | Qt::ItemIsUserCheckable );
		wdg->setCheckState( 0, Qt::Unchecked );
	}
	currentUpdateIdx = 0;
}
