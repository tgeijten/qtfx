#include "QLogSink.h"

#include <QAbstractScrollArea>
#include <QScrollBar>
#include "xo/string/string_tools.h"
#include "xo/system/assert.h"
#include "qtfx.h"
#include "xo/system/log_format.h"

QLogSink::QLogSink( QWidget* parent, xo::log::level level, xo::log::sink_mode mode ) :
	QPlainTextEdit( parent ),
	sink( level, {}, mode ),
	enabled_( true ),
	buffer_mutex_( QMutex::NonRecursive )
{
	setFont( getMonospaceFont( 9 ) );
	creation_thread_id_ = QThread::currentThreadId();
	connect( &update_timer_, &QTimer::timeout, this, &QLogSink::update );
	update_timer_.setInterval( 1000 );
	update_timer_.start();
}

void QLogSink::hande_log_message( xo::log::level l, const xo::string& msg )
{
	if ( QThread::currentThreadId() != creation_thread_id_ )
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
	xo_assert( QThread::currentThreadId() == creation_thread_id_ );

	buffer_mutex_.lock();
	for ( auto& e : buffer_data_ )
		append_message( e.first, e.second );
	buffer_data_.clear();
	buffer_mutex_.unlock();
}

void QLogSink::append_message( xo::log::level l, const xo::string& msg )
{
	moveCursor( QTextCursor::End );
	QTextCursor cursor( textCursor() );
	QTextCharFormat format;
	format.setFontWeight( QFont::Normal );
	format.setForeground( QBrush( Qt::black ) );

	switch ( l )
	{
	case xo::log::level::trace:
	case xo::log::level::debug:
		format.setForeground( QBrush( Qt::gray ) );
		break;
	case xo::log::level::info:
		format.setForeground( QBrush( textColor( Qt::blue ) ) );
		break;
	case xo::log::level::warning:
		format.setFontWeight( QFont::Bold );
		format.setForeground( QBrush( textColor( Qt::yellow ) ) );
		break;
	case xo::log::level::error:
	case xo::log::level::critical:
		format.setFontWeight( QFont::Bold );
		format.setForeground( QBrush( textColor( Qt::red ) ) );
		break;
	default:
		break;
	}

	cursor.setCharFormat( format );
	cursor.insertText( QString( xo::trim_right_str( msg ).c_str() ) + "\n" );
	verticalScrollBar()->setValue( verticalScrollBar()->maximum() );
}
