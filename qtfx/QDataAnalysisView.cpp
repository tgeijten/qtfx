#include "QDataAnalysisView.h"

#include <algorithm>

#include "qcustomplot/qcustomplot.h"
#include "QAction"
#include "QHeaderView"
#include "qtfx.h"
#include "qt_convert.h"
#include "gui_profiler.h"

#include "xo/container/container_tools.h"
#include "xo/container/sorted_vector.h"
#include "xo/numerical/bounds.h"
#include "xo/numerical/compare.h"
#include "xo/numerical/constants.h"
#include "xo/numerical/math.h"
#include "xo/system/log.h"
#include "xo/utility/color.h"
#include "xo/xo_types.h"

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel& m, QWidget* parent ) :
	QWidget( parent ),
	currentUpdateIdx( 0 ),
	model( m )
{
	GUI_PROFILE_FUNCTION;

	filter = new QLineEdit( this );
	filter->setPlaceholderText( "Filter channels" );
	connect( filter, &QLineEdit::textChanged, this, &QDataAnalysisView::filterChanged );

	selectBox = new QCheckBox( this );
	connect( selectBox, &QCheckBox::stateChanged, this, &QDataAnalysisView::select );

	filterGroup = new QHGroup( this, 0, 4 );
	*filterGroup << filter << selectBox;

	itemList = new QTreeWidget( this );
	itemList->setRootIsDecorated( false );
	itemList->setColumnCount( 2 );
	itemList->header()->close();
	itemList->resize( 100, 100 );
	QStringList headerLabels;
	headerLabels << "Variable" << "Value";
	itemList->setHeaderLabels( headerLabels );

	keepButton = new QPushButton( "&Keep Selected Graphs", this );
	connect( keepButton, &QPushButton::clicked, this, &QDataAnalysisView::holdSeries );

	itemGroup = new QVGroup( this, 0, 4 );
	itemGroup->setContentsMargins( 0, 0, 0, 0 );
	*itemGroup << filterGroup << itemList << keepButton;
	connect( itemList, &QTreeWidget::itemChanged, this, &QDataAnalysisView::itemChanged );

	splitter = new QSplitter( this );
	splitter->setContentsMargins( 0, 0, 0, 0 );
	splitter->setObjectName( "Analysis.Splitter" );
	splitter->addWidget( itemGroup );

	QVBoxLayout* layout = new QVBoxLayout( this );
	setLayout( layout );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->setSpacing( 4 );
	layout->addWidget( splitter );

	for ( int i = 0; i < maxSeriesCount; ++i )
		freeColors.insert( i );

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

	splitter->setSizes( QList< int >{ 100, 300 } );
	reloadData();
}

int QDataAnalysisView::decimalPoints( double v )
{
	if ( v != 0 && abs( v ) < 0.1 )
		return 6;
	else return 3;
}

void QDataAnalysisView::setTime( double time, bool refreshAll )
{
	GUI_PROFILE_FUNCTION;

	if ( itemList->topLevelItemCount() != model.channelCount() )
		return reloadData();

	if ( model.channelCount() == 0 )
		return;

	// update state
	currentTime = time;

	// draw stuff if visible
	if ( isVisible() )
	{
		int itemCount = refreshAll ? int( model.channelCount() ) : std::min<int>( smallRefreshItemCount, int( model.channelCount() ) );
		itemList->setUpdatesEnabled( false );
		for ( size_t i = 0; i < itemCount; ++i )
		{
			auto y = model.value( currentUpdateIdx, time );
			itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString::asprintf( "%.*f", decimalPoints( y ), y ) );
			++currentUpdateIdx %= model.channelCount();
		}
		itemList->setUpdatesEnabled( true );

		// update graph
		updateIndicator();
		if ( refreshAll )
			customPlot->replot( QCustomPlot::rpQueued );
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

void QDataAnalysisView::mouseEvent( QMouseEvent* e )
{
	if ( model.hasData() && ( e->buttons() & Qt::LeftButton ) != 0 )
	{
		double x = customPlot->xAxis->pixelToCoord( e->pos().x() );
		double t = model.timeValue( model.timeIndex( x ) );
		emit timeChanged( t );
	}
}

void QDataAnalysisView::rangeChanged( const QCPRange& newRange, const QCPRange& oldRange )
{
	if ( model.hasData() )
	{
		const auto modelRange = QCPRange( model.timeStart(), model.timeFinish() + 1e-4 );
		auto fixedRange = QCPRange( xo::max( newRange.lower, modelRange.lower ), xo::min( newRange.upper, modelRange.upper ) );
		if ( fixedRange.size() < modelRange.size() && model.frameCount() > minDataPointsVisible ) {
			auto minZoomRange = averageFrameDuration * minDataPointsVisible;
			if ( fixedRange.size() < minZoomRange ) {
				fixedRange.lower = fixedRange.center() - minZoomRange / 2;
				fixedRange.upper = fixedRange.lower + minZoomRange;
			}
			if ( fixedRange.lower < modelRange.lower )
				fixedRange += modelRange.lower - fixedRange.lower;
			if ( fixedRange.upper > modelRange.upper )
				fixedRange += modelRange.upper - fixedRange.upper;
		}
		else fixedRange = modelRange;

		if ( fixedRange != newRange )
		{
			customPlot->xAxis->blockSignals( true );
			customPlot->xAxis->setRange( fixedRange );
			customPlot->xAxis->blockSignals( false );
		}
		if ( autoFitVerticalAxis )
			fitVerticalAxis();
		updateSeriesStyle();
	}
}

void QDataAnalysisView::filterChanged( const QString& filter )
{
	updateFilter();
}

void QDataAnalysisView::setSelectionState( int state )
{
	if ( state != Qt::PartiallyChecked )
	{
		for ( int i = 0; i < itemList->topLevelItemCount(); ++i )
		{
			auto* item = itemList->topLevelItem( i );
			if ( !item->isHidden() && item->checkState( 0 ) != state )
				item->setCheckState( 0, Qt::CheckState( state ) );
		}
	}
}

void QDataAnalysisView::reloadData()
{
	GUI_PROFILE_FUNCTION;

	itemList->clear();
	clearSeries();

	for ( int i = 0; i < model.channelCount(); ++i )
	{
		auto* wdg = new QTreeWidgetItem( itemList, QStringList( model.label( i ) ) );
		wdg->setTextAlignment( 1, Qt::AlignRight );
		wdg->setFlags( wdg->flags() | Qt::ItemIsUserCheckable );
		wdg->setCheckState( 0, persistentSerieNames.find( model.label( i ) ) != persistentSerieNames.end() ? Qt::Checked : Qt::Unchecked );
	}
	itemList->resizeColumnToContents( 0 );

	currentUpdateIdx = 0;
	currentTime = 0.0;
	updateIndicator();
	updateFilter();
}

void QDataAnalysisView::setRange( double lower, double upper )
{
	customPlot->xAxis->setRange( lower, upper );
	customPlot->replot();
}

void QDataAnalysisView::updateIndicator()
{
	customPlotLine->start->setCoords( currentTime, customPlot->yAxis->range().lower );
	customPlotLine->end->setCoords( currentTime, customPlot->yAxis->range().upper );
}

void QDataAnalysisView::updateFilter()
{
	//selectAllButton->setDisabled( filter->text().isEmpty() );
	auto regexp = QRegExp( filter->text(), Qt::CaseSensitive, QRegExp::Wildcard );
	for ( int i = 0; i < itemList->topLevelItemCount(); ++i )
	{
		auto* item = itemList->topLevelItem( i );
		const auto& s = item->text( 0 );
		bool match = s.contains( filter->text() ) || regexp.exactMatch( s );
		item->setHidden( !match );
	}

	updateSelectBox();
}

void QDataAnalysisView::updateSelectBox()
{
	size_t checked_count = 0;
	size_t shown_count = 0;

	for ( int i = 0; i < itemList->topLevelItemCount(); ++i )
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

void QDataAnalysisView::fitVerticalAxis()
{
	//xo::bounds<double> yrange( xo::num<double>::max, xo::num<double>::lowest );
	xo::bounds<double> yrange( 0, 0 );
	auto xrange = customPlot->xAxis->range();
	int end_frame = model.timeIndex( xrange.upper );
	for ( int frame = model.timeIndex( xrange.lower ); frame <= end_frame; ++frame )
		for ( auto& s : series )
			yrange.extend( model.value( s.channel, frame ) );

	customPlot->yAxis->setRange( yrange.lower, yrange.upper );
}

void QDataAnalysisView::updateSeriesStyle()
{
	auto zoom = averageFrameDuration * customPlot->xAxis->axisRect()->width() / customPlot->xAxis->range().size();
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
			persistentSerieNames.insert( model.label( idx ) );
		}
		else item->setCheckState( 0, Qt::Unchecked );
	}
	else if ( series_it != series.end() && item->checkState( 0 ) == Qt::Unchecked )
	{
		removeSeries( idx );
		persistentSerieNames.remove( model.label( idx ) );
	}
	updateSelectBox();
}

void QDataAnalysisView::addSeries( int idx )
{
	GUI_PROFILE_FUNCTION;

	QCPGraph* graph = customPlot->addGraph();
	QString name = model.label( idx );
	graph->setName( name );

	xo_assert( !freeColors.empty() );

	auto color = to_qt( xo::make_unique_color( freeColors.front() ) );
	graph->setScatterStyle( QCPScatterStyle( seriesStyle == discStyle ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone, 4 ) );
	graph->setPen( QPen( color, lineWidth ) );

	for ( int frame = 0; frame < model.frameCount(); ++frame )
		graph->addData( model.timeValue( frame ), model.value( idx, frame ) );
	if ( model.frameCount() > 0 )
		averageFrameDuration = ( model.timeFinish() - model.timeStart() ) / model.frameCount();
	else averageFrameDuration = 0.0f;

	series.emplace_back( Series{ idx, freeColors.front(), graph } );
	freeColors.erase( freeColors.begin() );

	updateSeriesStyle();

	auto range = customPlot->xAxis->range();
	customPlot->rescaleAxes();
	customPlot->xAxis->setRange( range );
	updateIndicator();
	customPlot->replot();
}

void QDataAnalysisView::removeSeries( int idx )
{
	GUI_PROFILE_FUNCTION;

	auto range = customPlot->xAxis->range();
	auto it = xo::find_if( series, [&]( auto& p ) { return idx == p.channel; } );

	freeColors.insert( it->color );
	if ( it->graph )
		customPlot->removeGraph( it->graph );
	series.erase( it );

	customPlot->rescaleAxes();
	customPlot->xAxis->setRange( range );
	updateIndicator();
	customPlot->replot();
}

void QDataAnalysisView::holdSeries()
{
	GUI_PROFILE_FUNCTION;

	// remove existing 
	for ( auto* g : heldSeries )
		customPlot->removeGraph( g );
	heldSeries.clear();

	// copy from series
	for ( auto& s : series )
	{
		auto* graph = customPlot->addGraph();
		graph->setData( s.graph->data(), true );
		graph->setName( s.graph->name() );
		graph->setPen( QPen( s.graph->pen().color().lighter(), lineWidth ) );
		heldSeries.push_back( graph );
	}

	customPlot->replot();
}
