#include "qtfx.h"

#include <QBoxLayout>
#include <QApplication>
#include <QColor>

QWidget* createVBoxWidget( QWidget* parent, int margin, int spacing )
{
	QWidget* w = new QWidget( parent );
	QVBoxLayout* l = new QVBoxLayout;
	l->setContentsMargins( margin, margin, margin, margin );
	l->setSpacing( spacing );
	l->addWidget( w );
	return w;
}

QWidget* createHBoxWidget( QWidget* parent, int margin, int spacing )
{
	QWidget* w = new QWidget( parent );
	QHBoxLayout* l = new QHBoxLayout;
	l->setContentsMargins( margin, margin, margin, margin );
	l->setSpacing( spacing );
	l->addWidget( w );
	return w;
}

QFont getMonospaceFont( int pointSize, int weight )
{
#if defined(_MSC_VER)
	return QFont( "Consolas", pointSize, weight );
#elif defined(__APPLE__)
	return QFont( "Menlo", 3 * pointSize / 2, weight );
#else
	return QFont( "Consolas", pointSize, weight );
#endif
}

void cycleTabWidget( QTabWidget* wdg, int ofs )
{
	if ( wdg->count() > 1 )
	{
		int newIdx = ( wdg->currentIndex() + ofs ) % wdg->count();
		if ( newIdx < 0 )
			newIdx += wdg->count();
		wdg->setCurrentIndex( newIdx );
	}
}

bool darkMode()
{
	return QApplication::palette().background().color().valueF() < 0.5f;
}

Qt::GlobalColor textColor( Qt::GlobalColor c )
{
	if ( darkMode() ) {
		if ( c == Qt::blue )
			return Qt::white; // even light blue looks bad on a dark background
		else return c;
	}
	else return Qt::GlobalColor( c + 6 );
}
