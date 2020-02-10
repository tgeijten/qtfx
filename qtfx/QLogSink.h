#pragma once

#include <QPlainTextEdit>
#include <QThread>
#include <QTimer>
#include <QMutex>

#include "xo/system/log_sink.h"
#include "xo/xo_types.h"

class QLogSink : public QPlainTextEdit, public xo::log::sink
{
	Q_OBJECT

public:
	QLogSink( QWidget* parent, xo::log::level level = xo::log::level::info, xo::log::sink_mode mode = xo::log::sink_mode::all_threads );
	virtual ~QLogSink() {}

	/// set different log_level for non-ui threads
	void set_thread_log_level( xo::log::level l ) { thread_log_level_ = l; }

public slots:
	void update();

protected:
	virtual void hande_log_message( xo::log::level l, const xo::string& msg ) override;

private:
	void append_message( xo::log::level l, const xo::string& msg );
	bool enabled_;
	Qt::HANDLE creation_thread_id_;
	xo::log::level thread_log_level_;

	std::vector< std::pair< xo::log::level, std::string > > buffer_data_;
	QMutex buffer_mutex_;
	QTimer update_timer_;
};
