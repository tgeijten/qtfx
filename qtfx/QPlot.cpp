#include "QPlot.h"

QPlot::QPlot( QWidget* parent )
{
#if defined QTFX_USE_QCUSTOMPLOT
	customPlot = new QCustomPlot();
	customPlot->setInteraction( QCP::iRangeZoom, true );
	customPlot->setInteraction( QCP::iRangeDrag, true );
	customPlot->axisRect()->setRangeDrag( Qt::Horizontal );
	customPlot->axisRect()->setRangeZoom( Qt::Horizontal );
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
#endif
}

QPlot::~QPlot()
{

}
