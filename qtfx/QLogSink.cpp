#include "QLogSink.h"

#include <QAbstractScrollArea>
#include <QScrollBar>

QLogSink::QLogSink( QWidget* parent, flut::log::level level ) :
QPlainTextEdit( parent ),
sink( level ),
enabled_( true ),
buffer_mutex_( QMutex::NonRecursive )
{
	qt_thread_id_ = QThread::currentThreadId();
	connect( &update_timer_, &QTimer::timeout, this, &QLogSink::update );
	update_timer_.setInterval( 1000 );
	update_timer_.start();
}

void QLogSink::send_log_message( flut::log::level l, const flut::string& msg )
{
	if ( QThread::currentThreadId() != qt_thread_id_ )
	{
		// accessed from a different thread, add to buffer and wait for update
		buffer_mutex_.lock();
		buffer_data_.push_back( std::make_pair( l, msg ) );
		buffer_mutex_.unlock();
		
	}
	else append_message( l, msg );
}

void QLogSink::update()
{
	flut_assert( QThread::currentThreadId() == qt_thread_id_ );

	buffer_mutex_.lock();
	for ( auto& e : buffer_data_ )
		append_message( e.first, e.second );
	buffer_data_.clear();
	buffer_mutex_.unlock();
}

void QLogSink::append_message( flut::log::level l, const flut::string& msg )
{
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
