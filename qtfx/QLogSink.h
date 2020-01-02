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
	QLogSink( QWidget* parent = 0, xo::log::level level = xo::log::level::info );
	virtual ~QLogSink() {}

	virtual void send_log_message( xo::log::level l, const xo::string& msg ) override;

	bool enabled() const { return enabled_; }
	void enabled( bool val ) { enabled_ = val; }

public slots:
	void update();

private:
	void append_message( xo::log::level l, const xo::string& msg );
	bool enabled_;
	Qt::HANDLE qt_thread_id_;

	std::vector< std::pair< xo::log::level, std::string > > buffer_data_;
	QMutex buffer_mutex_;
	QTimer update_timer_;
};
