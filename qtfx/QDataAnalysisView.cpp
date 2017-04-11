#include "QDataAnalysisView.h"
#include "QAction"
#include "QHeaderView"
#include "QtWidgets/QGraphicsLayout"
#include <algorithm>
#include "simvis/color.h"
#include "flut/system/log_sink.hpp"
#include "flut/system/types.hpp"
#include <set>
#include "qtfx.h"

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent ) : QWidget( parent ), model( m ), currentUpdateIdx( 0 )
{
	selectAll = new QCheckBox( this );
	selectAll->setText( "Select All" );
	selectAll->setDisabled( true );
	connect( selectAll, &QCheckBox::stateChanged, this, &QDataAnalysisView::selectAllChanged );

	filter = new QLineEdit( this );
	connect( filter, &QLineEdit::textChanged, this, &QDataAnalysisView::filterChanged );

	auto* header = new QHGroup( this, 0, 4 );
	*header << filter << selectAll;

	itemList = new QTreeWidget( this );
	itemList->setRootIsDecorated( false );
	itemList->setColumnCount( 2 );
	itemList->header()->close();
	itemList->resize( 100, 100 );
	QStringList headerLabels;
	headerLabels << "Variable" << "Value";
	itemList->setHeaderLabels( headerLabels );

	itemGroup = new QVGroup( this, 0, 4 );
	*itemGroup << header << itemList;

	//itemList->header()->setStretchLastSection( false );

	connect( itemList, &QTreeWidget::itemChanged, this, &QDataAnalysisView::itemChanged );

	splitter = new QSplitter( this );
	splitter->setContentsMargins( 0, 0, 0, 0 );
	splitter->setObjectName( "Analysis.Splitter" );
	splitter->addWidget( itemGroup );

	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setContentsMargins( 4, 4, 4, 4 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );

#ifdef QTFX_USE_QCUSTOMPLOT
	customPlot = new QCustomPlot();
	//customPlot->setInteraction( QCP::iRangeDrag, true );
	customPlot->setInteraction( QCP::iRangeZoom, true );
	customPlot->axisRect()->setRangeDrag( Qt::Horizontal );
	customPlot->axisRect()->setRangeZoom( Qt::Horizontal );
	customPlot->legend->setVisible( true );
	customPlot->legend->setFont( itemList->font() );
	customPlot->legend->setRowSpacing( -6 );
	customPlotLine = new QCPItemLine( customPlot );
	customPlotLine->setHead( QCPLineEnding( QCPLineEnding::esDiamond, 9, 9, true ) );
	customPlotLine->setTail( QCPLineEnding( QCPLineEnding::esDiamond, 9, 9, true ) );
	customPlot->addItem( customPlotLine );
	splitter->addWidget( customPlot );
	connect( customPlot, &QCustomPlot::mousePress, this, &QDataAnalysisView::mouseEvent );
	connect( customPlot, &QCustomPlot::mouseMove, this, &QDataAnalysisView::mouseEvent );
#else
	chart = new QtCharts::QChart();
	chart->setBackgroundRoundness( 0 );
	chart->setMargins( QMargins( 4, 4, 4, 4 ) );
	chart->layout()->setContentsMargins( 0, 0, 0, 0 );
	chart->legend()->setAlignment( Qt::AlignRight );
	chart->createDefaultAxes();
	chartView = new QtCharts::QChartView( chart, this );
	chartView->setContentsMargins( 0, 0, 0, 0 );
	chartView->setRenderHint( QPainter::Antialiasing );
	chartView->setRubberBand( QtCharts::QChartView::RectangleRubberBand );
	chartView->setBackgroundBrush( QBrush( Qt::red ) );
	chartView->resize( 300, 100 );
	splitter->addWidget( chartView );
#endif

	splitter->setSizes( QList< int >{ 100, 300 } );
	reset();
}

void QDataAnalysisView::refresh( double time, bool refreshAll )
{
	if ( itemList->topLevelItemCount() != model->getVariableCount() )
		return reset();

	if ( model->getVariableCount() == 0 )
		return;

	// update state
	currentTime = time;

	// draw stuff if visible
	if ( isVisible() )
	{
		int itemCount = refreshAll ? model->getVariableCount() : std::min<int>( smallRefreshItemCount, model->getVariableCount() );
		itemList->setUpdatesEnabled( false );
		for ( size_t i = 0; i < itemCount; ++i )
		{
			itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString().sprintf( "%.3f", model->getValue( currentUpdateIdx, time ) ) );
			++currentUpdateIdx %= model->getVariableCount();
		}
		itemList->setUpdatesEnabled( true );

		// update graph
		updateIndicator();

#ifdef QTFX_USE_QCUSTOMPLOT
		customPlot->replot( QCustomPlot::rpQueued );
#endif
	}
}

void QDataAnalysisView::itemChanged( QTreeWidgetItem* item, int column )
{
	if ( column == 0 )
		updateSeries( itemList->indexOfTopLevelItem( item ) );
}

void QDataAnalysisView::clearSeries()
{
	while ( !series.empty() )
		removeSeries( series.back().first );
}

void QDataAnalysisView::mouseEvent( QMouseEvent* event )
{
#ifdef QTFX_USE_QCUSTOMPLOT
	if ( event->buttons() & Qt::LeftButton )
	{
		double x = customPlot->xAxis->pixelToCoord( event->pos().x() );
		flut::math::clamp( x, model->getTimeStart(), model->getTimeFinish() );
		emit timeChanged( x );
	}
#endif
}

void QDataAnalysisView::filterChanged( const QString& filter )
{
	updateFilter();
}

void QDataAnalysisView::selectAllChanged( int state )
{
	if ( state != Qt::PartiallyChecked )
	{
		for ( size_t i = 0; i < itemList->topLevelItemCount(); ++i )
		{
			auto* item = itemList->topLevelItem( i );
			if ( !item->isHidden() && item->checkState( 0 ) != state )
				item->setCheckState( 0, Qt::CheckState( state ) );
		}
	}
}

QColor QDataAnalysisView::getStandardColor( int idx )
{
	int a = idx % 3;
	int b = ( idx / 3 ) % 3;
	int hidx = a * 120 + b * 45;

	float hue = fmod( hidx, 360. );
	vis::color c = vis::make_from_hsv( hue, 1.0f, 0.75f );
	return QColor( 255 * c.r, 255 * c.g, 255 * c.b );
}

void QDataAnalysisView::reset()
{
	// remove series, keep names
	std::set< QString > keep_series;
	for ( size_t i = 0; i < itemList->topLevelItemCount(); ++i )
	{
		auto* item = itemList->topLevelItem( i );
		if ( item->checkState( 0 ) == Qt::Checked )
			keep_series.insert( item->text( 0 ) );
	}

	itemList->clear();
	clearSeries();

	for ( size_t i = 0; i < model->getVariableCount(); ++i )
	{
		auto* wdg = new QTreeWidgetItem( itemList, QStringList( model->getLabel( i ) ) );
		wdg->setTextAlignment( 1, Qt::AlignRight );
		wdg->setFlags( wdg->flags() | Qt::ItemIsUserCheckable );
		wdg->setCheckState( 0, keep_series.count( model->getLabel( i ) ) > 0 ? Qt::Checked : Qt::Unchecked );
	}
	itemList->resizeColumnToContents( 0 );

	currentUpdateIdx = 0;
	currentTime = 0.0;
	updateIndicator();
	updateFilter();
}

void QDataAnalysisView::updateIndicator()
{
#ifdef QTFX_USE_QCUSTOMPLOT
	customPlotLine->start->setCoords( currentTime, customPlot->yAxis->range().lower );
	customPlotLine->end->setCoords( currentTime, customPlot->yAxis->range().upper );
	customPlot->replot();
#else
	auto pos = chart->mapToPosition( QPointF( currentTime, 0 ) );
#endif
}

void QDataAnalysisView::updateFilter()
{
	selectAll->setDisabled( filter->text().isEmpty() );

	for ( size_t i = 0; i < itemList->topLevelItemCount(); ++i )
	{
		auto* item = itemList->topLevelItem( i );
		item->setHidden( !item->text( 0 ).contains( filter->text() ) );
	}
}

void QDataAnalysisView::updateSeries( int idx )
{
	auto item = itemList->topLevelItem( idx );
	auto series_it = series.find( idx );
	if ( item->checkState( 0 ) == Qt::Checked && series_it == series.end() )
		addSeries( idx );
	else if ( series_it != series.end() && item->checkState( 0 ) == Qt::Unchecked )
		removeSeries( idx );
}

void QDataAnalysisView::addSeries( int idx )
{
#ifdef QTFX_USE_QCUSTOMPLOT
	QCPGraph* graph = customPlot->addGraph();
	graph->setName( model->getLabel( idx ) );
	auto data = model->getSeries( idx, minSeriesInterval );
	for ( auto& e : data )
		graph->addData( e.first, e.second );
	series[ idx ] = graph;
	int i = 0;
	for ( auto& s : series )
		s.second->setPen( QPen( getStandardColor( i++ ) ) );

	auto range = customPlot->xAxis->range();
	customPlot->rescaleAxes();
	customPlot->xAxis->setRange( range );
	updateIndicator();
	customPlot->replot();
#else
	QtCharts::QLineSeries* ls = new QtCharts::QLineSeries;
	ls->setName( model->getLabel( idx ) );
	auto data = model->getSeries( idx, minSeriesInterval );
	for ( auto& e : data )
		ls->append( e.first, e.second );
	chart->addSeries( ls );
	chart->createDefaultAxes();
	chart->zoomReset();
	series[ idx ] = ls;
#endif
}

void QDataAnalysisView::removeSeries( int idx )
{
#ifdef QTFX_USE_QCUSTOMPLOT
	auto it = series.find( idx );
	customPlot->removeGraph( it->second );
	customPlot->rescaleAxes();
	customPlot->replot();
	series.erase( it );
	updateIndicator();
#else
	auto it = series.find( idx );
	chart->removeSeries( it->second );
	chart->zoomReset();
	chart->createDefaultAxes();
	series.erase( it );
#endif
}