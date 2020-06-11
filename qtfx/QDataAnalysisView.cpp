#include "QDataAnalysisView.h"

#include "QAction"
#include "QHeaderView"
#include <algorithm>
#include "xo/utility/color.h"
#include "xo/system/log.h"
#include "xo/xo_types.h"
#include "qtfx.h"
#include <array>
#include "xo/system/log.h"
#include "xo/container/sorted_vector.h"
#include "xo/numerical/constants.h"
#include "qt_convert.h"
#include "xo/numerical/compare.h"
#include "xo/numerical/math.h"
#include "xo/container/container_tools.h"
#include "qcustomplot/qcustomplot.h"

QDataAnalysisView::QDataAnalysisView( QDataAnalysisModel* m, QWidget* parent ) :
	QWidget( parent ),
	currentUpdateIdx( 0 ),
	model( m )
{
	filter = new QLineEdit( this );
	filter->setPlaceholderText( "Filter channels" );
	connect( filter, &QLineEdit::textChanged, this, &QDataAnalysisView::filterChanged );

	selectBox = new QCheckBox( this );
	connect( selectBox, &QCheckBox::stateChanged, this, &QDataAnalysisView::select );

	auto* header = new QHGroup( this, 0, 4 );
	*header << filter << selectBox;

	itemList = new QTreeWidget( this );
	itemList->setRootIsDecorated( false );
	itemList->setColumnCount( 2 );
	itemList->header()->close();
	itemList->resize( 100, 100 );
	QStringList headerLabels;
	headerLabels << "Variable" << "Value";
	itemList->setHeaderLabels( headerLabels );

	itemGroup = new QVGroup( this, 0, 4 );
	itemGroup->setContentsMargins( 0, 0, 0, 0 );
	*itemGroup << header << itemList;
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
	reset();
}

int QDataAnalysisView::decimalPoints( double v )
{
	if ( v != 0 && xo::less_than_or_equal( abs( v ), 0.05 ) )
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
		int itemCount = refreshAll ? int( model->seriesCount() ) : std::min<int>( smallRefreshItemCount, int( model->seriesCount() ) );
		itemList->setUpdatesEnabled( false );
		for ( size_t i = 0; i < itemCount; ++i )
		{
			auto y = model->value( currentUpdateIdx, time );
			itemList->topLevelItem( currentUpdateIdx )->setText( 1, QString::asprintf( "%.*f", decimalPoints( y ), y ) );
			++currentUpdateIdx %= model->seriesCount();
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
	if ( model->hasData() && ( e->buttons() & Qt::LeftButton ) != 0 )
	{
		double x = customPlot->xAxis->pixelToCoord( e->pos().x() );
		double t = model->timeValue( model->timeIndex( x ) );
		emit timeChanged( t );
	}
}

void QDataAnalysisView::rangeChanged( const QCPRange &newRange, const QCPRange &oldRange )
{
	if ( model->hasData() )
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

void QDataAnalysisView::reset()
{
	itemList->clear();
	clearSeries();

	for ( int  i = 0; i < model->seriesCount(); ++i )
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
	customPlotLine->start->setCoords( currentTime, customPlot->yAxis->range().lower );
	customPlotLine->end->setCoords( currentTime, customPlot->yAxis->range().upper );
}

void QDataAnalysisView::updateFilter()
{
	//selectAllButton->setDisabled( filter->text().isEmpty() );
	for ( int i = 0; i < itemList->topLevelItemCount(); ++i )
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
	QCPGraph* graph = customPlot->addGraph();
	QString name = model->label( idx );
	graph->setName( name );

	xo_assert( !freeColors.empty() );

	auto color = to_qt( xo::make_unique_color( freeColors.front() ) );
	graph->setScatterStyle( QCPScatterStyle( seriesStyle == discStyle ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone, 4 ) );
	graph->setPen( QPen( color, lineWidth ) );

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
}

void QDataAnalysisView::removeSeries( int idx )
{
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
