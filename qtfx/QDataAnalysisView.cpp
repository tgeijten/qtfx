#include "QDataAnalysisView.h"
#include "QAction"
#include "QHeaderView"
#include <algorithm>
#include "simvis/color.h"
#include "xo/system/log.h"
#include "xo/utility/types.h"
#include "qtfx.h"
#include <array>
#include "xo/system/log.h"
#include "xo/container/sorted_vector.h"
#include "xo/numerical/constants.h"
#include "qt_convert.h"

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent ) :
	QWidget( parent ),
	model( m ),
	currentUpdateIdx( 0 )
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
	//itemList->setFrameStyle( QFrame::NoFrame );

	itemGroup = new QVGroup( this, 0, 4 );
	itemGroup->setContentsMargins( 0, 0, 0, 0 );
	*itemGroup << header << itemList;
	connect( itemList, &QTreeWidget::itemChanged, this, &QDataAnalysisView::itemChanged );

	splitter = new QSplitter( this );
	splitter->setContentsMargins( 0, 0, 0, 0 );
	splitter->setFrameShape( QFrame::NoFrame );
	splitter->setObjectName( "Analysis.Splitter" );
	splitter->addWidget( itemGroup );

	QVBoxLayout* layout = new QVBoxLayout( this );
	setLayout( layout );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );

	for ( int i = 0; i < maxSeriesCount; ++i )
		freeColors.insert( i );

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

int QDataAnalysisView::decimalPoints( double v )
{
	if ( v != 0 && xo::less_or_equal( abs( v ), 0.05 ) )
		return 6;
	else return 3;
}

void QDataAnalysisView::refresh( double time, bool refreshAll )
{
	if ( itemList->topLevelItemCount() != model->seriesCount() )
		return reset();

	if ( model->seriesCount() == 0 )
		return;

	// update state
	currentTime = time;

	// draw stuff if visible
	if ( isVisible() )
	{
		int itemCount = refreshAll ? model->seriesCount() : std::min<int>( smallRefreshItemCount, model->seriesCount() );
		itemList->setUpdatesEnabled( false );
		for ( size_t i = 0; i < itemCount; ++i )
		{
			auto y = model->value( currentUpdateIdx, time );
			itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString().sprintf( "%.*f", decimalPoints( y ), y ) );
			++currentUpdateIdx %= model->seriesCount();
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
		removeSeries( series.rbegin()->channel );
}

void QDataAnalysisView::mouseEvent( QMouseEvent* event )
{
#if !defined QTFX_NO_QCUSTOMPLOT
	if ( event->buttons() & Qt::LeftButton )
	{
		double x = customPlot->xAxis->pixelToCoord( event->pos().x() );
		double t = model->timeValue( model->timeIndex( x ) );
		emit timeChanged( t );
	}
#endif
}

void QDataAnalysisView::rangeChanged( const QCPRange &newRange, const QCPRange &oldRange )
{
	QCPRange fixedRange = QCPRange( xo::max( newRange.lower, model->timeStart() ), xo::min( newRange.upper, model->timeFinish() ) );
	if ( fixedRange != newRange )
	{
		customPlot->xAxis->blockSignals( true );
		customPlot->xAxis->setRange( fixedRange );
		customPlot->xAxis->blockSignals( false );
	}

	updateSeriesStyle();
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

QColor QDataAnalysisView::getStandardColor( int idx )
{
	return to_qt( vis::make_unique_color( size_t( idx ) ) );
}

void QDataAnalysisView::reset()
{
	itemList->clear();
	clearSeries();

	for ( size_t i = 0; i < model->seriesCount(); ++i )
	{
		auto* wdg = new QTreeWidgetItem( itemList, QStringList( model->label( i ) ) );
		wdg->setTextAlignment( 1, Qt::AlignRight );
		wdg->setFlags( wdg->flags() | Qt::ItemIsUserCheckable );
		wdg->setCheckState( 0, persistentSerieNames.find( model->label( i ) ) != persistentSerieNames.end() ? Qt::Checked : Qt::Unchecked );
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

void QDataAnalysisView::updateSeriesStyle()
{
	auto zoom = currentSeriesInterval * customPlot->xAxis->axisRect()->width() / customPlot->xAxis->range().size();
	SeriesStyle newstyle = zoom > 8 ? discStyle : lineStyle;

	if ( newstyle != seriesStyle )
	{
		seriesStyle = newstyle;
		QCPScatterStyle ss = QCPScatterStyle( seriesStyle == discStyle ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone, 4 );
		for ( auto& s : series )
		{
			s.graph->setScatterStyle( ss );
			s.graph->setLineStyle( QCPGraph::lsLine );
		}
	}
}

void QDataAnalysisView::updateSeries( int idx )
{
	auto item = itemList->topLevelItem( idx );
	auto series_it = xo::find_if( series, [&]( auto& p ) { return idx == p.channel; } );
	if ( item->checkState( 0 ) == Qt::Checked && series_it == series.end() )
	{
		if ( series.size() < maxSeriesCount )
		{
			addSeries( idx );
			persistentSerieNames.insert( model->label( idx ) );
		}
		else item->setCheckState( 0, Qt::Unchecked );
	}
	else if ( series_it != series.end() && item->checkState( 0 ) == Qt::Unchecked )
	{
		removeSeries( idx );
		persistentSerieNames.remove( model->label( idx ) );
	}
	updateSelectBox();
}

void QDataAnalysisView::addSeries( int idx )
{
#if !defined QTFX_NO_QCUSTOMPLOT
	QCPGraph* graph = customPlot->addGraph();
	QString name = model->label( idx );
	graph->setName( name );

	xo_assert( !freeColors.empty() );

	graph->setScatterStyle( QCPScatterStyle( seriesStyle == discStyle ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone, 4 ) );
	graph->setPen( QPen( getStandardColor( freeColors.front() ), lineWidth ) );

	auto data = model->getSeries( idx, minSeriesInterval );
	for ( auto& e : data )
		graph->addData( e.first, e.second );

	series.emplace_back( Series{ idx, freeColors.front(), graph } );
	freeColors.erase( freeColors.begin() );

	currentSeriesInterval = ( data.back().first - data.front().first ) / data.size();

	updateSeriesStyle();

	auto range = customPlot->xAxis->range();
	customPlot->rescaleAxes();
	customPlot->xAxis->setRange( range );
	updateIndicator();
	customPlot->replot();

#else
	QtCharts::QLineSeries* ls = new QtCharts::QLineSeries;
	ls->setName( model->label( idx ) );
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
	auto it = xo::find_if( series, [&]( auto& p ) { return idx == p.channel; } );
	auto name = it->graph->name();

	freeColors.insert( it->color );

	customPlot->removeGraph( it->graph );
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
