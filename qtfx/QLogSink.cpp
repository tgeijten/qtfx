#include "QLogSink.h"

#include <QAbstractScrollArea>
#include <QScrollBar>

QLogSink::QLogSink( QWidget* parent, flut::log::level level ) :
QPlainTextEdit( parent ),
sink( level ),
enabled_( true )
{
}

void QLogSink::send_log_message( flut::log::level l, const flut::string& msg )
{
	// TODO: make thread-safe
	if ( !enabled() )
		return;

	moveCursor( QTextCursor::End );
	QTextCursor cursor( textCursor() );
	QTextCharFormat format;
	format.setFontWeight( QFont::Normal );
	format.setForeground( QBrush( Qt::black ) );

	switch ( l )
	{
	case flut::log::trace_level:
	case flut::log::debug_level:
		format.setForeground( QBrush( Qt::gray ) );
		break;
	case flut::log::info_level:
		format.setForeground( QBrush( Qt::darkBlue ) );
		break;
	case flut::log::warning_level:
		format.setFontWeight( QFont::DemiBold );
		format.setForeground( QBrush( Qt::darkYellow ) );
		break;
	case flut::log::error_level:
	case flut::log::critical_level:
		format.setFontWeight( QFont::DemiBold );
		format.setForeground( QBrush( Qt::darkRed ) );
		break;
	default:
		break;
	}

	cursor.setCharFormat( format );
	cursor.insertText( QString( flut::trim_right_str( msg ).c_str() ) + "\n" );
	verticalScrollBar()->setValue( verticalScrollBar()->maximum() );
}
