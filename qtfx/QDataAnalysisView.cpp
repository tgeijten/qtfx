#include "QDataAnalysisView.h"
#include "QAction"
#include "QHeaderView"
#include <algorithm>
#include "simvis/color.h"
#include "xo/system/log_sink.h"
#include "xo/utility/types.h"
#include <set>
#include "qtfx.h"
#include <array>
#include "xo/system/log.h"

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent ) : QWidget( parent ), model( m ), currentUpdateIdx( 0 )
{
	selectBox = new QCheckBox( this );
	connect( selectBox, &QCheckBox::stateChanged, this, &QDataAnalysisView::select );

	filter = new QLineEdit( this );
	connect( filter, &QLineEdit::textChanged, this, &QDataAnalysisView::filterChanged );

	auto* header = new QHGroup( this, 0, 4 );
	*header << new QLabel( "Filter", this ) << filter << selectBox;

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
	connect( itemList, &QTreeWidget::itemChanged, this, &QDataAnalysisView::itemChanged );

	splitter = new QSplitter( this );
	splitter->setContentsMargins( 0, 0, 0, 0 );
	splitter->setObjectName( "Analysis.Splitter" );
	splitter->addWidget( itemGroup );

	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setContentsMargins( 2, 0, 2, 0 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );

#if !defined QTFX_NO_QCUSTOMPLOT
	customPlot = new QCustomPlot();
	customPlot->setInteraction( QCP::iRangeZoom, true );
	customPlot->setInteraction( QCP::iRangeDrag, true );
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
	connect( customPlot->xAxis, SIGNAL( rangeChanged( const QCPRange&, const QCPRange& ) ), this, SLOT( rangeChanged( const QCPRange&, const QCPRange& ) ) );

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
	if ( itemList->topLevelItemCount() != model->getSeriesCount() )
		return reset();

	if ( model->getSeriesCount() == 0 )
		return;

	// update state
	currentTime = time;

	// draw stuff if visible
	if ( isVisible() )
	{
		int itemCount = refreshAll ? model->getSeriesCount() : std::min<int>( smallRefreshItemCount, model->getSeriesCount() );
		itemList->setUpdatesEnabled( false );
		for ( size_t i = 0; i < itemCount; ++i )
		{
			itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString().sprintf( "%.3f", model->getValue( currentUpdateIdx, time ) ) );
			++currentUpdateIdx %= model->getSeriesCount();
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
		removeSeries( series.rbegin()->first );
}

void QDataAnalysisView::mouseEvent( QMouseEvent* event )
{
#if !defined QTFX_NO_QCUSTOMPLOT
	if ( event->buttons() & Qt::LeftButton )
	{
		double x = customPlot->xAxis->pixelToCoord( event->pos().x() );
		xo::clamp( x, model->getTimeStart(), model->getTimeFinish() );
		emit timeChanged( x );
	}
#endif
}

void QDataAnalysisView::rangeChanged( const QCPRange &newRange, const QCPRange &oldRange )
{
	auto newZoom = currentSeriesInterval * customPlot->xAxis->axisRect()->width() / newRange.size();
	auto oldZoom = currentSeriesInterval * customPlot->xAxis->axisRect()->width() / oldRange.size();
	if ( ( newZoom > 5 && oldZoom <= 5 ) || ( oldZoom > 5 && newZoom <= 5 ) )
		refreshSeriesStyle();
}

void QDataAnalysisView::filterChanged( const QString& filter )
{
	updateFilter();
}

void QDataAnalysisView::setSelectionState( int state )
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

QColor QDataAnalysisView::getStandardColor( int idx, float value )
{
	static std::array< float, 10 > standard_hue{ 0, 60, 120, 180, 240, 300, 30, 210, 270, 330 };
	static std::array< float, 10 > standard_val{ 1, 0.75, 0.75, 0.75, 1, 1, 1, 0.75, 1, 1 };
	float hue = standard_hue[ idx % standard_hue.size() ];
	float sat = 1.0f / ( 1.0f + idx / standard_hue.size() );
	float val = standard_val[ idx % standard_hue.size() ];
	vis::color c = vis::make_from_hsv( hue, sat, val );
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

	for ( size_t i = 0; i < model->getSeriesCount(); ++i )
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
#if !defined QTFX_NO_QCUSTOMPLOT
	customPlotLine->start->setCoords( currentTime, customPlot->yAxis->range().lower );
	customPlotLine->end->setCoords( currentTime, customPlot->yAxis->range().upper );
	customPlot->replot();
#else
	auto pos = chart->mapToPosition( QPointF( currentTime, 0 ) );
#endif
}

void QDataAnalysisView::updateFilter()
{
	//selectAllButton->setDisabled( filter->text().isEmpty() );
	for ( size_t i = 0; i < itemList->topLevelItemCount(); ++i )
	{
		auto* item = itemList->topLevelItem( i );
		item->setHidden( !item->text( 0 ).contains( filter->text() ) );
	}

	updateSelectBox();
}

void QDataAnalysisView::updateSelectBox()
{
	size_t checked_count = 0;
	size_t shown_count = 0;

	for ( size_t i = 0; i < itemList->topLevelItemCount(); ++i )
	{
		auto* item = itemList->topLevelItem( i );
		if ( !item->isHidden() )
		{
			shown_count++;
			checked_count += item->checkState( 0 ) == Qt::Checked;
		}
	}

	selectBox->blockSignals( true );
	selectBox->setCheckState( checked_count > 0 ? ( shown_count == checked_count ? Qt::Checked : Qt::Checked ) : Qt::Unchecked );
	selectBox->blockSignals( false );
}

void QDataAnalysisView::refreshSeriesStyle()
{
	auto zoom = currentSeriesInterval * customPlot->xAxis->axisRect()->width() / customPlot->xAxis->range().size();
	QCPScatterStyle ss = QCPScatterStyle( zoom > 5 ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone, 5 );

	int i = 0;
	for ( auto& s : series )
	{
		s.second->setPen( QPen( getStandardColor( i++ ) ) );
		s.second->setScatterStyle( ss );
		s.second->setLineStyle( QCPGraph::lsLine );
	}
}

void QDataAnalysisView::updateSeries( int idx )
{
	auto item = itemList->topLevelItem( idx );
	auto series_it = series.find( idx );
	if ( item->checkState( 0 ) == Qt::Checked && series_it == series.end() )
	{
		if ( series.size() < maxSeriesCount )
			addSeries( idx );
		else item->setCheckState( 0, Qt::Unchecked );
	}
	else if ( series_it != series.end() && item->checkState( 0 ) == Qt::Unchecked )
	{
		removeSeries( idx );
	}
	updateSelectBox();
}

void QDataAnalysisView::addSeries( int idx )
{
#if !defined QTFX_NO_QCUSTOMPLOT
	QCPGraph* graph = customPlot->addGraph();
	graph->setName( model->getLabel( idx ) );
	auto data = model->getSeries( idx, minSeriesInterval );
	for ( auto& e : data )
		graph->addData( e.first, e.second );
	series[ idx ] = graph;
	currentSeriesInterval = ( data.back().first - data.front().first ) / data.size();

	refreshSeriesStyle();

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
#if !defined QTFX_NO_QCUSTOMPLOT
	auto range = customPlot->xAxis->range();
	auto it = series.find( idx );

	customPlot->removeGraph( it->second );
	series.erase( it );

	customPlot->rescaleAxes();
	customPlot->xAxis->setRange( range );
	updateIndicator();
	customPlot->replot();
#else
	auto it = series.find( idx );
	chart->removeSeries( it->second );
	chart->zoomReset();
	chart->createDefaultAxes();
	series.erase( it );
#endif
}
